// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <cmath>
#include <memory>
#include <string>
#include <vector>

#include "eckit/geo/Grid.h"
#include "eckit/geo/Point.h"
#include "eckit/testing/Test.h"

#include "mir/api/MIRJob.h"
#include "mir/data/MIRField.h"
#include "mir/input/GribMemoryInput.h"
#include "mir/key/grid/Grid.h"
#include "mir/output/ArrayOutput.h"
#include "mir/output/GribMemoryOutput.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/util/BoundingBox.h"
#include "mir/util/MeshGeneratorParameters.h"

#include "repres.h"


namespace mir::repres::tests {


// a key and its value (long if integral)
struct kv_t {
    std::string key;
    double value;
};


// a point (latitude, longitude)
struct point_t {
    double lat;
    double lon;
};


using keys_t   = std::vector<kv_t>;
using points_t = std::vector<point_t>;


keys_t operator+(keys_t a, const keys_t& b) {
    a.insert(a.end(), b.begin(), b.end());
    return a;
}


/// GRIB2 message of a gridType (by sample) and its keys, of n values (the point index), or (n = 0) of a single value
/// for Nx * Ny points
std::string message(const std::string& gridType, const keys_t& keys, long n = 0) {
    std::unique_ptr<codes_handle, decltype(&codes_handle_delete)> h(
        codes_grib_handle_new_from_samples(nullptr, "GRIB2"), &codes_handle_delete);
    ASSERT(h);

    GRIB_CALL(codes_set_string(h.get(), "gridType", gridType.c_str(), nullptr));

    for (const auto& kv : keys) {
        if (kv.value == std::round(kv.value)) {
            GRIB_CALL(codes_set_long(h.get(), kv.key.c_str(), static_cast<long>(kv.value)));
        }
        else {
            GRIB_CALL(codes_set_double(h.get(), kv.key.c_str(), kv.value));
        }
    }

    std::vector<double> v(n > 0 ? static_cast<size_t>(n) : 1, 1.);
    for (size_t i = 0; n > 0 && i < v.size(); ++i) {
        v[i] = static_cast<double>(i);
    }

    if (n == 0) {
        long nx = 0;
        long ny = 0;
        GRIB_CALL(codes_get_long(h.get(), "Nx", &nx));
        GRIB_CALL(codes_get_long(h.get(), "Ny", &ny));
        n = nx * ny;
    }

    // as grib_filter: values before the number of points

    GRIB_CALL(codes_set_double_array(h.get(), "values", v.data(), v.size()));
    GRIB_CALL(codes_set_long(h.get(), "numberOfDataPoints", n));
    GRIB_CALL(codes_set_long(h.get(), "numberOfValues", n));

    const void* msg = nullptr;
    size_t length   = 0;
    GRIB_CALL(codes_get_message(h.get(), &msg, &length));

    return {static_cast<const char*>(msg), length};
}


RepresentationHandle representation(const std::string& msg) {
    input::GribMemoryInput input(msg.data(), msg.size());
    const input::MIRInput& in = input;
    return in.field().representation();
}


points_t points(const Representation& repres) {
    points_t visited;
    for (std::unique_ptr<Iterator> it(repres.iterator()); it->next();) {
        const auto& p = it->pointUnrotated();
        visited.push_back({p.lat().value(), p.lon().value()});
    }
    return visited;
}


/// Representation of a typed grid (gridType=...;key=value;...)
RepresentationHandle typed(const std::string& grid) {
    const auto& g = key::grid::Grid::lookup(grid);

    param::SimpleParametrisation param;
    g.parametrisation(grid, param);

    return g.representation(param);
}


const std::string LAEA =
    "gridType=lambert_azimuthal_equal_area;grid=100000/100000;standardParallelInDegrees=52;"
    "centralLongitudeInDegrees=10;latitudeOfFirstGridPointInDegrees=66.982143;"
    "longitudeOfFirstGridPointInDegrees=-35.034024;Ni=40;Nj=38";

const std::string POLAR_STEREOGRAPHIC =
    "gridType=polar_stereographic;LaDInDegrees=60;orientationOfTheGridInDegrees=0;southPoleOnProjectionPlane=0;"
    "grid=100000/100000;latitudeOfFirstGridPointInDegrees=60;longitudeOfFirstGridPointInDegrees=-45;Ni=20;Nj=20";

const std::string LAMBERT =
    "gridType=lambert;LaDInDegrees=50;LoVInDegrees=10;Latin1InDegrees=50;Latin2InDegrees=50;grid=100000/100000;"
    "latitudeOfFirstGridPointInDegrees=40;longitudeOfFirstGridPointInDegrees=-10;Ni=20;Nj=15";


CASE("projected grids (GRIB) by their gridSpec: points as PROJ") {
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

    struct test_t {
        std::string name;
        std::string gridType;
        keys_t keys;
        size_t size;
        points_t points;  // first, second and last
    };

    const std::vector<test_t> tests{
        {"transverse_mercator",
         "transverse_mercator",
         TMERC +
             keys_t{{"X1", 50000000}, {"Y1", 20000000}, {"X2", 50600000}, {"Y2", 20600000}, {"jScansPositively", 1}},
         4 * 3,
         {{51.689376316, -0.553164312}, {51.689016444, -0.524237842}, {51.742209034, -0.464560974}}},
        {"transverse_mercator (iScansNegatively, jScansPositively=0)",
         "transverse_mercator",
         TMERC + keys_t{{"X1", 50600000},
                        {"Y1", 20600000},
                        {"X2", 50000000},
                        {"Y2", 20000000},
                        {"iScansNegatively", 1},
                        {"jScansPositively", 0}},
         4 * 3,
         {{51.742209034, -0.464560974}, {51.742583866, -0.493520620}, {51.689376316, -0.553164312}}},
        {"albers", "albers", AEA, 31 * 29, {{23., 264.}, {22.998841507, 264.478562795}, {34.484152764, 280.544471606}}},
    };

    for (const auto& test : tests) {
        SECTION(test.name) {
            RepresentationHandle repres(
                representation(message(test.gridType, test.keys, static_cast<long>(test.size))));

            EXPECT_EQUAL(repres->numberOfPoints(), test.size);
            EXPECT_EQUAL(repres->grid().type(), "regular_xy");

            const auto visited = points(*repres);
            ASSERT(visited.size() == test.size);

            const std::vector<point_t> cases{visited[0], visited[1], visited[test.size - 1]};
            for (size_t i = 0; i < cases.size(); ++i) {
                const auto& p = cases[i];
                const auto& q = test.points[i];

                // longitudes as fractions (see LongitudeFraction), with a relative precision of ~1e-7
                EXPECT(eckit::geo::points_equal(eckit::geo::PointLonLat{p.lon, p.lat},
                                                eckit::geo::PointLonLat{q.lon, q.lat}, 1e-4));
            }

            // decoded only: no GRIB encoding (of the grid)
            grib_info info;
            EXPECT_THROWS_AS(repres->fillGrib(info), exception::FunctionalityNotSupported);
        }
    }


    SECTION("transverse_mercator: interpolation from, and GRIB re-encoding") {
        const auto msg = message(tests.front().gridType, tests.front().keys, 4 * 3);

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
        input::GribMemoryInput same_input(msg.data(), msg.size());
        reencode.execute(same_input, grib);

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
}


CASE("space_view (GRIB): points as the scanning, element diagonal") {
    // sectors of GRIB2 space_view (Nr=6.6107, dx=3622), points as the scanning (first, second and last)
    const keys_t sector{{"Nx", 16},      {"Ny", 16},   {"dx", 3622}, {"dy", 3622},   {"Xp", 1856000},
                        {"Yp", 1856000}, {"Xo", 1500}, {"Yo", 1700}, {"Nr", 6610700}};

    struct test_t {
        std::string name;
        keys_t keys;
        points_t points;
    };

    const std::vector<test_t> tests{
        {"shapeOfTheEarth=6",
         {{"shapeOfTheEarth", 6}},
         {{4.223693723, 350.299524805}, {4.223622147, 350.327193534}, {3.815601110, 350.719698129}}},
        {"shapeOfTheEarth=6, iScansNegatively, jScansPositively",
         {{"shapeOfTheEarth", 6}, {"iScansNegatively", 1}, {"jScansPositively", 1}},
         {{-4.223693724, 9.700475199}, {-4.223622148, 9.672806469}, {-3.815601111, 9.280301874}}},
        {"shapeOfTheEarth=5, sub-satellite point",
         {{"shapeOfTheEarth", 5},
          {"longitudeOfSubSatellitePoint", -75000000},
          {"Ny", 12},
          {"dy", 3610},
          {"Xo", 2400},
          {"Yo", 1200}},
         {{18.425011761, 300.905565350}, {18.425558651, 300.936080607}, {18.106822780, 301.327135202}}},
    };

    for (const auto& test : tests) {
        SECTION(test.name) {
            RepresentationHandle repres(representation(message("space_view", sector + test.keys)));

            const auto visited = points(*repres);
            EXPECT_EQUAL(visited.size(), repres->numberOfPoints());

            const std::vector<point_t> cases{visited.front(), visited[1], visited.back()};
            for (size_t i = 0; i < cases.size(); ++i) {
                const auto& p = cases[i];
                const auto& q = test.points[i];

                // longitudes are fractions (mir::Longitude), which approximate (to about 1e-5 degrees)
                EXPECT(std::abs(p.lat - q.lat) < 1e-6);
                EXPECT(std::abs(std::remainder(p.lon - q.lon, 360.)) < 1e-5);
            }

            double d = 0.;
            EXPECT(repres->getLongestElementDiagonal(d));
            EXPECT(d > 0.);

            grib_info info;
            EXPECT_THROWS_AS(repres->fillGrib(info), exception::FunctionalityNotSupported);
        }
    }
}


CASE("typed grids (lambert, lambert_azimuthal_equal_area, polar_stereographic)") {
    struct test_t {
        std::string grid;
        size_t Ni;
        size_t Nj;
        long grid_type;
        double latitudeOfFirstGridPoint;
        double longitudeOfFirstGridPoint;  // as GRIB2, positive
    };

    const std::vector<test_t> tests{
        {LAMBERT, 20, 15, CODES_UTIL_GRID_SPEC_LAMBERT_CONFORMAL, 40., 350.},
        {LAEA, 40, 38, CODES_UTIL_GRID_SPEC_LAMBERT_AZIMUTHAL_EQUAL_AREA, 66.982143, -35.034024},
        {POLAR_STEREOGRAPHIC, 20, 20, CODES_UTIL_GRID_SPEC_POLAR_STEREOGRAPHIC, 60., 315.},
    };

    for (const auto& test : tests) {
        SECTION(test.grid) {
            RepresentationHandle repres(typed(test.grid));

            const auto n = test.Ni * test.Nj;
            EXPECT_EQUAL(repres->numberOfPoints(), n);
            EXPECT_EQUAL(count(*repres), n);
            EXPECT_EQUAL(static_cast<size_t>(repres->atlasGrid().size()), n);

            EXPECT(repres->sameAs(*typed(test.grid)));
            EXPECT(repres->uniqueName().find("RegularXY-") == 0);
            EXPECT(to_string(*repres).find("RegularXY[") == 0);

            EXPECT_NOT(repres->isGlobal());
            EXPECT_NOT(repres->includesSouthPole());
            if (test.grid != POLAR_STEREOGRAPHIC) {
                EXPECT_NOT(repres->includesNorthPole());
                EXPECT_NOT(repres->isPeriodicWestEast());
            }

            util::MeshGeneratorParameters mesh;
            repres->fillMeshGen(mesh);
            EXPECT_EQUAL(mesh.meshGenerator_, "structured");

            // the first point (top left), and the scanning
            std::unique_ptr<Iterator> it(repres->iterator());
            ASSERT(it->next());
            EXPECT(same(it->pointUnrotated().lat().value(), test.latitudeOfFirstGridPoint, 1e-6));

            grib_info info;
            repres->fillGrib(info);

            EXPECT_EQUAL(info.grid.grid_type, test.grid_type);
            EXPECT_EQUAL(info.packing.editionNumber, 2);
            EXPECT_EQUAL(info.grid.Ni, static_cast<long>(test.Ni));
            EXPECT_EQUAL(info.grid.Nj, static_cast<long>(test.Nj));
            EXPECT(same(info.grid.latitudeOfFirstGridPointInDegrees, test.latitudeOfFirstGridPoint, 1e-6));
            EXPECT(same(info.grid.longitudeOfFirstGridPointInDegrees, test.longitudeOfFirstGridPoint, 1e-6));
            EXPECT_EQUAL(info.grid.iScansNegatively, 0);
            EXPECT_EQUAL(info.grid.jScansPositively, 0);

            EXPECT(same(extra_double(info, "DxInMetres"), 100000., 1e-3));
            EXPECT(same(extra_double(info, "DyInMetres"), 100000., 1e-3));
            EXPECT_EQUAL(extra_long(info, "shapeOfTheEarth"), 6);  // the default figure (sphere, R=6371229m)
        }
    }


    SECTION("lambert: GRIB keys") {
        grib_info info;
        typed(LAMBERT)->fillGrib(info);

        EXPECT(same(extra_double(info, "Latin1InDegrees"), 50.));
        EXPECT(same(extra_double(info, "Latin2InDegrees"), 50.));
        EXPECT(same(extra_double(info, "LoVInDegrees"), 10.));
        EXPECT(same(extra_double(info, "LaDInDegrees"), 50.));
        EXPECT(same(info.grid.latitudeOfSouthernPoleInDegrees, -90.));
        EXPECT(same(info.grid.longitudeOfSouthernPoleInDegrees, 0.));
        EXPECT_EQUAL(info.grid.uvRelativeToGrid, 0);
    }


    SECTION("lambert_azimuthal_equal_area: GRIB keys") {
        grib_info info;
        typed(LAEA)->fillGrib(info);

        EXPECT(same(extra_double(info, "standardParallelInDegrees"), 52.));
        EXPECT(same(extra_double(info, "centralLongitudeInDegrees"), 10.));
    }


    SECTION("polar_stereographic: GRIB keys, and the south pole on the projection plane") {
        grib_info info;
        typed(POLAR_STEREOGRAPHIC)->fillGrib(info);

        EXPECT(same(extra_double(info, "orientationOfTheGridInDegrees"), 0.));
        EXPECT(same(extra_double(info, "LaDInDegrees"), 60.));

        RepresentationHandle south(
            typed("gridType=polar_stereographic;LaDInDegrees=-60;orientationOfTheGridInDegrees=-90;"
                  "southPoleOnProjectionPlane=1;grid=100000/100000;latitudeOfFirstGridPointInDegrees=-60;"
                  "longitudeOfFirstGridPointInDegrees=-45;Ni=20;Nj=20"));

        grib_info s;
        south->fillGrib(s);
        EXPECT(same(s.grid.latitudeOfFirstGridPointInDegrees, -60., 1e-6));
        EXPECT(same(extra_double(s, "orientationOfTheGridInDegrees"), 270.));
        EXPECT(south->boundingBox().north() < Latitude::EQUATOR);
    }


    SECTION("typed grids: errors") {
        // a key not of the type
        EXPECT_THROWS_AS(typed(LAMBERT + ";unknown=1"), exception::UserError);

        // not a key=value pair
        EXPECT_THROWS_AS(typed(LAMBERT + ";Ni"), exception::UserError);

        // a required key missing
        const auto& g = key::grid::Grid::lookup(LAMBERT);
        param::SimpleParametrisation param;
        g.parametrisation(LAMBERT, param);
        param.clear("LaDInDegrees");
        EXPECT_THROWS_AS(g.representation(param), exception::UserError);
    }
}


CASE("RegularXY: GRIB encoding of the figure, and by edition") {
    const std::string R_GRIB1 = ";radius=6367470";


    SECTION("GRIB1: only the GRIB1 figure (spherical)") {
        RepresentationHandle grib1(typed(LAMBERT + R_GRIB1));

        grib_info info;
        info.packing.editionNumber = 1;
        grib1->fillGrib(info);

        EXPECT_EQUAL(extra_long(info, "earthIsOblate"), 0);
        EXPECT(extra(info, "shapeOfTheEarth") == nullptr);
        EXPECT(extra(info, "LaDInDegrees") == nullptr);  // GRIB1 cannot write LaD
        EXPECT(same(info.grid.longitudeOfFirstGridPointInDegrees, -10., 1e-6));

        // the edition, as an option
        grib_info option;
        option.extra_set("edition", 1L);
        grib1->fillGrib(option);
        EXPECT_EQUAL(extra_long(option, "earthIsOblate"), 0);

        // other figures
        RepresentationHandle other(typed(LAMBERT));

        grib_info error;
        error.packing.editionNumber = 1;
        EXPECT_THROWS_AS(other->fillGrib(error), exception::FunctionalityNotSupported);
    }


    SECTION("GRIB1: positive longitudes (writeLonPositive)") {
        RepresentationHandle repres(typed(LAMBERT + R_GRIB1 + ";writeLonPositive=1;writeLaDInDegrees=0"));

        grib_info info;
        info.packing.editionNumber = 1;
        repres->fillGrib(info);

        EXPECT(same(info.grid.longitudeOfFirstGridPointInDegrees, 350., 1e-6));
    }


    SECTION("GRIB2: shape of the Earth, by the figure") {
        struct test_t {
            std::string figure;
            long shapeOfTheEarth;
        };

        const std::vector<test_t> tests{
            {R_GRIB1, 0},
            {";radius=6370000", 1},
            {";earthMajorAxis=6378160;earthMinorAxis=6356775", 2},
            {";earthMajorAxis=6378000;earthMinorAxis=6357000", 7},
        };

        for (const auto& test : tests) {
            grib_info info;
            typed(LAMBERT + test.figure)->fillGrib(info);

            EXPECT_EQUAL(extra_long(info, "shapeOfTheEarth"), test.shapeOfTheEarth);

            if (test.shapeOfTheEarth == 1) {
                EXPECT(same(extra_double(info, "radius"), 6370000.));
            }

            if (test.shapeOfTheEarth == 7) {
                EXPECT(same(extra_double(info, "earthMajorAxis"), 6378000.));
                EXPECT(same(extra_double(info, "earthMinorAxis"), 6357000.));
            }
        }
    }


    SECTION("first point bottom-left") {
        param::SimpleParametrisation param;
        param.set("gridType", "lambert");
        param.set("LaDInDegrees", 50.);
        param.set("LoVInDegrees", 10.);
        param.set("grid", std::vector<double>{100000., 100000.});
        param.set("latitudeOfFirstGridPointInDegrees", 40.);
        param.set("longitudeOfFirstGridPointInDegrees", -10.);
        param.set("Ni", 20L);
        param.set("Nj", 15L);
        param.set("first_point_bottom_left", true);
        param.set("uvRelativeToGrid", 1L);

        RepresentationHandle repres(RepresentationFactory::build(param));
        EXPECT_EQUAL(repres->numberOfPoints(), 20 * 15);

        grib_info info;
        repres->fillGrib(info);

        EXPECT(same(info.grid.latitudeOfFirstGridPointInDegrees, 40., 1e-6));
        EXPECT(same(info.grid.longitudeOfFirstGridPointInDegrees, 350., 1e-6));
        EXPECT_EQUAL(info.grid.jScansPositively, 1);
        EXPECT_EQUAL(info.grid.uvRelativeToGrid, 1);

        // the projection parameters are required
        param.clear("LaDInDegrees");
        EXPECT_THROWS_AS(RepresentationFactory::build(param), exception::UserError);
    }
}


CASE("RegularXY: by PROJ string, or by gridSpec only") {
    SECTION("by PROJ string (decoded only)") {
        param::SimpleParametrisation param;
        param.set("gridType", "regular_xy");
        param.set("proj", "+proj=merc +datum=WGS84 +units=m");
        param.set("grid", std::vector<double>{100000., 100000.});
        param.set("latitudeOfFirstGridPointInDegrees", 10.);
        param.set("longitudeOfFirstGridPointInDegrees", 0.);
        param.set("Ni", 3L);
        param.set("Nj", 2L);

        RepresentationHandle repres(RepresentationFactory::build(param));
        EXPECT_EQUAL(repres->numberOfPoints(), 6);
        EXPECT_EQUAL(count(*repres), 6);

        grib_info info;
        EXPECT_THROWS_AS(repres->fillGrib(info), exception::FunctionalityNotSupported);
    }


    SECTION("by gridSpec only") {
        param::SimpleParametrisation param;
        param.set("gridType", "mercator");
        param.set("Ni", 3L);
        param.set("Nj", 2L);

        EXPECT_THROWS_AS(RepresentationFactory::build(param), exception::FunctionalityNotSupported);
    }
}


CASE("RegularXY: named grids (grids.yaml)") {
    // NOTE: the "-proj" named grids (by PROJ string) cannot be built, lacking the projection keys (see report)
    const auto& grid = key::grid::Grid::lookup("LAEA-EFAS-5km");
    EXPECT_EQUAL(grid.gaussianNumber(), 1280);
    EXPECT_EQUAL(grid.gridname(), "LAEA-EFAS-5km");

    RepresentationHandle repres(grid.representation());
    EXPECT_EQUAL(repres->numberOfPoints(), 1000 * 950);

    grib_info info;
    repres->fillGrib(info);
    EXPECT_EQUAL(info.grid.grid_type, CODES_UTIL_GRID_SPEC_LAMBERT_AZIMUTHAL_EQUAL_AREA);
    EXPECT_EQUAL(info.grid.Ni, 1000);
    EXPECT_EQUAL(info.grid.Nj, 950);
    EXPECT(same(info.grid.latitudeOfFirstGridPointInDegrees, 66.982143, 1e-6));
    EXPECT_EQUAL(extra_long(info, "shapeOfTheEarth"), 6);
}


}  // namespace mir::repres::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
