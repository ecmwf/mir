// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <cmath>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "eckit/geo/Grid.h"
#include "eckit/geo/Point.h"

#include "mir/api/MIRJob.h"
#include "mir/data/MIRField.h"
#include "mir/input/GribMemoryInput.h"
#include "mir/input/MIRInput.h"
#include "mir/output/ArrayOutput.h"
#include "mir/output/GribMemoryOutput.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Grib.h"

#include "eckit/testing/Test.h"


namespace mir::tests::unit {


// GRIB2 projected grids supported by their gridSpec (ecCodes), points as PROJ (first, second and last)
using keys_t   = std::vector<std::pair<std::string, double>>;
using points_t = std::vector<std::pair<double, double>>;  // (lat, lon)


struct test_t {
    std::string gridType;
    keys_t keys;
    points_t points;
};


// transverse_mercator (template 3.12, British National Grid), albers (template 3.31, GRS80)
const keys_t TMERC{{"Ni", 4},
                   {"Nj", 3},
                   {"shapeOfTheEarth", 7},
                   {"scaleFactorOfEarthMajorAxis", 2},
                   {"scaledValueOfEarthMajorAxis", 637756340},
                   {"scaleFactorOfEarthMinorAxis", 2},
                   {"scaledValueOfEarthMinorAxis", 635625691},
                   {"latitudeOfReferencePoint", 49000000},
                   {"longitudeOfReferencePoint", -2000000},
                   {"scaleFactorAtReferencePoint", 0.9996012717},
                   {"XR", 40000000},
                   {"YR", -10000000},
                   {"Di", 200000},
                   {"Dj", 300000}};


const keys_t AEA{{"Nx", 31},
                 {"Ny", 29},
                 {"shapeOfTheEarth", 4},
                 {"latitudeOfFirstGridPointInDegrees", 23},
                 {"longitudeOfFirstGridPointInDegrees", 264},
                 {"LaDInDegrees", 23},
                 {"LoVInDegrees", 264},
                 {"Latin1InDegrees", 29.5},
                 {"Latin2InDegrees", 45.5},
                 {"DxInMetres", 50000},
                 {"DyInMetres", 50000},
                 {"jScansPositively", 1}};


keys_t operator+(keys_t a, const keys_t& b) {
    a.insert(a.end(), b.begin(), b.end());
    return a;
}


const std::vector<test_t> TESTS{
    {"transverse_mercator",
     TMERC + keys_t{{"X1", 50000000}, {"Y1", 20000000}, {"X2", 50600000}, {"Y2", 20600000}, {"jScansPositively", 1}},
     {{51.689376316, -0.553164312}, {51.689016444, -0.524237842}, {51.742209034, -0.464560974}}},
    {"transverse_mercator",
     TMERC + keys_t{{"X1", 50600000},
                    {"Y1", 20600000},
                    {"X2", 50000000},
                    {"Y2", 20000000},
                    {"iScansNegatively", 1},
                    {"jScansPositively", 0}},
     {{51.742209034, -0.464560974}, {51.742583866, -0.493520620}, {51.689376316, -0.553164312}}},
    {"albers", AEA, {{23., 264.}, {22.998841507, 264.478562795}, {34.484152764, 280.544471606}}},
};


std::string message(const test_t& test, size_t& size) {
    std::unique_ptr<codes_handle, decltype(&codes_handle_delete)> h(
        codes_grib_handle_new_from_samples(nullptr, "GRIB2"), &codes_handle_delete);
    ASSERT(h);

    GRIB_CALL(codes_set_string(h.get(), "gridType", test.gridType.c_str(), nullptr));

    for (const auto& kv : test.keys) {
        const auto& key  = kv.first;
        const auto value = kv.second;
        if (value == std::round(value)) {
            GRIB_CALL(codes_set_long(h.get(), key.c_str(), static_cast<long>(value)));
        }
        else {
            GRIB_CALL(codes_set_double(h.get(), key.c_str(), value));
        }
    }

    const auto n = static_cast<long>(test.gridType == "albers" ? 31 * 29 : 4 * 3);
    GRIB_CALL(codes_set_long(h.get(), "numberOfDataPoints", n));
    GRIB_CALL(codes_set_long(h.get(), "numberOfValues", n));

    std::vector<double> values(n);
    for (size_t i = 0; i < values.size(); ++i) {
        values[i] = static_cast<double>(i);
    }
    GRIB_CALL(codes_set_double_array(h.get(), "values", values.data(), values.size()));

    const void* msg = nullptr;
    size_t length   = 0;
    GRIB_CALL(codes_get_message(h.get(), &msg, &length));

    size = static_cast<size_t>(n);
    return {static_cast<const char*>(msg), length};
}


CASE("projected grids (GRIB) by their gridSpec: points as PROJ") {
    for (const auto& test : TESTS) {
        SECTION(test.gridType) {
            size_t size    = 0;
            const auto msg = message(test, size);

            std::unique_ptr<input::MIRInput> input(new input::GribMemoryInput(msg.data(), msg.size()));
            const auto field = input->field();
            repres::RepresentationHandle repres(field.representation());

            EXPECT_EQUAL(repres->numberOfPoints(), size);
            EXPECT_EQUAL(repres->grid().type(), "regular_xy");

            points_t visited;
            for (std::unique_ptr<repres::Iterator> it(repres->iterator()); it->next();) {
                const auto& p = it->pointUnrotated();
                visited.emplace_back(p.lat().value(), p.lon().value());
            }

            ASSERT(visited.size() == size);
            // visited point (by index) and reference point (by index)
            struct case_t {
                size_t visited;
                size_t reference;
            };

            const std::vector<case_t> cases{{0, 0}, {1, 1}, {size - 1, 2}};
            for (const auto& c : cases) {
                const auto& p = visited[c.visited];
                const auto& q = test.points[c.reference];

                // longitudes as fractions (see LongitudeFraction), with a relative precision of ~1e-7
                EXPECT(eckit::geo::points_equal(eckit::geo::PointLonLat{p.second, p.first},
                                                eckit::geo::PointLonLat{q.second, q.first}, 1e-4));
            }
        }
    }
}


CASE("transverse_mercator: interpolation from, and GRIB re-encoding") {
    size_t size    = 0;
    const auto msg = message(TESTS.front(), size);

    // interpolation, to a grid within
    api::MIRJob job;
    job.set("grid", std::vector<double>{0.01, 0.01});
    job.set("area", std::vector<double>{51.74, -0.55, 51.69, -0.47});
    job.set("interpolation", "nn");

    output::ArrayOutput array;
    input::GribMemoryInput input(msg.data(), msg.size());
    job.execute(input, array);
    EXPECT_EQUAL(array.size(), 6 * 9);

    // re-encoding (packing), the grid as is
    api::MIRJob reencode;
    reencode.set("accuracy", 12L);

    std::vector<char> buffer(1024 * 1024);
    output::GribMemoryOutput grib(buffer.data(), buffer.size());
    input::GribMemoryInput same(msg.data(), msg.size());
    reencode.execute(same, grib);

    std::unique_ptr<codes_handle, decltype(&codes_handle_delete)> h(
        codes_handle_new_from_message(nullptr, buffer.data(), grib.length()), &codes_handle_delete);
    ASSERT(h);

    char gridType[64];
    size_t len = sizeof(gridType);
    GRIB_CALL(codes_get_string(h.get(), "gridType", gridType, &len));
    EXPECT_EQUAL(std::string(gridType), "transverse_mercator");

    long bits = 0;
    GRIB_CALL(codes_get_long(h.get(), "bitsPerValue", &bits));
    EXPECT_EQUAL(bits, 12);
}


}  // namespace mir::tests::unit


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
