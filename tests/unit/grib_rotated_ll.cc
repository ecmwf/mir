// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <cstdio>
#include <memory>
#include <vector>

#include "eccodes.h"

#include "eckit/geo/Grid.h"
#include "eckit/geo/Point.h"
#include "eckit/spec/Custom.h"
#include "eckit/testing/Test.h"

#include "mir/util/Exceptions.h"


namespace mir::tests::unit {


CASE("rotated regular alt/lon grids") {
    using eckit::geo::PointLonLat;
    using eckit::geo::points_equal;


    constexpr double EPS = 1e-3;

    std::unique_ptr<FILE, decltype(&std::fclose)> file(std::fopen("rotated_ll.3-3.grib1", "rb"), &std::fclose);
    ASSERT(file);

    int err = 0;
    std::unique_ptr<codes_handle, decltype(&codes_handle_delete)> h(
        codes_grib_handle_new_from_file(nullptr, file.get(), &err), &codes_handle_delete);
    ASSERT(err == CODES_SUCCESS && h);

    auto get = [&h](const char* key) {
        double value = 0;
        CODES_CHECK(codes_get_double(h.get(), key, &value), nullptr);
        return value;
    };

    // grid is described in the rotated frame, MARS-like description
    std::vector<double> rotation{get("longitudeOfSouthernPoleInDegrees"), get("latitudeOfSouthernPoleInDegrees")};
    std::vector<double> grid{get("iDirectionIncrementInDegrees"), get("jDirectionIncrementInDegrees")};
    std::vector<double> area{get("latitudeOfFirstGridPointInDegrees"), get("longitudeOfFirstGridPointInDegrees"),
                             get("latitudeOfLastGridPointInDegrees"), get("longitudeOfLastGridPointInDegrees")};

    std::unique_ptr<const eckit::geo::Grid> g(
        eckit::geo::GridFactory::build(eckit::spec::Custom{{"grid", grid}, {"area", area}, {"rotation", rotation}}));
    ASSERT(g);

    EXPECT_EQUAL(g->size(), static_cast<size_t>(get("numberOfDataPoints")));


    SECTION("first points") {
        std::unique_ptr<codes_iterator, decltype(&codes_grib_iterator_delete)> it(
            codes_grib_iterator_new(h.get(), 0, &err), &codes_grib_iterator_delete);
        ASSERT(err == CODES_SUCCESS && it);

        for (const auto& p : std::vector<PointLonLat>{{-130.936, -18.156}, {-131.649, -17.323}}) {
            double lat   = 0;
            double lon   = 0;
            double value = 0;
            EXPECT(codes_grib_iterator_next(it.get(), &lat, &lon, &value) != 0);
            EXPECT(points_equal(PointLonLat{lon, lat}, p, EPS));
        }
    }


    SECTION("iterators agree") {
        std::unique_ptr<codes_iterator, decltype(&codes_grib_iterator_delete)> it(
            codes_grib_iterator_new(h.get(), 0, &err), &codes_grib_iterator_delete);
        ASSERT(err == CODES_SUCCESS && it);

        auto git = g->cbegin();

        double lat   = 0;
        double lon   = 0;
        double value = 0;
        for (size_t i = 0; i < 10; ++i, ++git) {
            EXPECT(codes_grib_iterator_next(it.get(), &lat, &lon, &value) != 0);
            EXPECT(points_equal(*git, PointLonLat{lon, lat}, EPS));
        }
    }
}


}  // namespace mir::tests::unit


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
