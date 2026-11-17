// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <sstream>
#include <string>
#include <vector>

#include "interpolate.h"


namespace mir::action::tests {


CASE("interpolate.grid2gridspec") {
    struct test_t {
        std::string grid;
        size_t numberOfPoints;
        std::string uniqueName;  // prefix
    };

    const std::vector<test_t> tests{
        test_t{"{grid: O8}", 544, "R8-"},
        test_t{"O8", 544, "R8-"},
        test_t{"{grid: F8}", 512, "F8-"},
        test_t{"{grid: [10, 10]}", 19 * 36, "LL-10x10-90:0:-90:350"},
        test_t{"{grid: [10, 10], area: [60, -10, 30, 40]}", 4 * 6, "LL-10x10-60:-10:30:40"},
        test_t{"{grid: [10, 10], rotation: [-40, 22]}", 19 * 36, "LL-10x10-90:0:-90:350-rot:-40:22:0"},
        test_t{"{grid: H4}", 192, "H4"},
        test_t{"{grid: H4, order: nested}", 192, "HN4"},
    };
    for (const auto& test : tests) {
        SECTION(test.grid) {
            auto a   = make_checked("{type: interpolate.grid2gridspec, grid: " + test.grid + "}");
            auto out = perform(*a, field("{grid: O16}", LATITUDE, 2));

            EXPECT_EQUAL(out.dimensions(), 2);
            EXPECT_EQUAL(out.values(0).size(), test.numberOfPoints);
            EXPECT_EQUAL(out.representation()->numberOfPoints(), test.numberOfPoints);
            EXPECT(out.representation()->uniqueName().find(test.uniqueName) == 0);

            EXPECT(!out.hasMissing());
            EXPECT(max_error(out, LATITUDE, 80.) < 0.2);
            EXPECT(contained(out, a->outputBoundingBox()));
        }
    }
}


CASE("interpolate.grid2gridspec, values") {
    const std::vector<std::string> grids{"{grid: O8}", "{grid: [10, 10], area: [60, -10, 30, 40]}", "{grid: H4}"};
    for (const std::string& grid : grids) {
        SECTION(grid) {
            check_values(*make_checked("{type: interpolate.grid2gridspec, grid: " + grid + "}"));
        }
    }
}


CASE("interpolate.grid2gridspec, HEALPix methods") {
    struct test_t {
        std::string interpolation;
        double tolerance;
    };

    // H4 latitude spacing is about 12 degrees
    const std::vector<test_t> tests{
        test_t{"healpix-resample", 1.},
        test_t{"healpix-statistics, interpolation-statistics: mean", 1.},
        test_t{"healpix-statistics", 7.},
        test_t{"healpix-statistics, interpolation-statistics: minimum", 7.},
    };
    for (const auto& test : tests) {
        SECTION(test.interpolation) {
            auto a =
                make("{type: interpolate.grid2gridspec, grid: {grid: H4}, interpolation: " + test.interpolation + "}");
            EXPECT_EQUAL(make(a->json_str(false))->json_str(false), a->json_str(false));

            auto constant = perform(*a, field("{grid: H8}", CONSTANT));

            EXPECT_EQUAL(constant.values(0).size(), 192);
            EXPECT(max_error(constant, CONSTANT) < 1e-9);

            auto linear = perform(*a, field("{grid: H8}", LATITUDE));

            EXPECT(max_error(linear, LATITUDE) < test.tolerance);
        }
    }
}


CASE("interpolate.grid2gridspec, cropping") {
    auto a = make_mutable("{type: interpolate.grid2gridspec, grid: {grid: O8}}");

    EXPECT(!a->canCrop());

    EXPECT(a->mergeWithNext(*make("{type: crop.area, area: [60, -10, 30, 40]}")));
    EXPECT(a->canCrop());
    EXPECT(a->outputBoundingBox() == util::BoundingBox(60, -10, 30, 40));

    auto out = perform(*a, field("{grid: O16}", LATITUDE));

    EXPECT(0 < out.values(0).size() && out.values(0).size() < 544);
    EXPECT(contained(out, a->outputBoundingBox()));
    EXPECT(max_error(out, LATITUDE) < 0.2);
}


CASE("interpolate.grid2gridspec, properties") {
    auto a = make_checked("{type: interpolate.grid2gridspec, grid: {grid: [10, 10], area: [60, -10, 30, 40]}}");

    EXPECT(a->isRegridAction());
    EXPECT(!a->canCrop());

    std::string name;

    EXPECT(!a->getGriddedTargetName(name));

    EXPECT_EQUAL(a->type(), "interpolate.grid2gridspec");
    EXPECT_EQUAL(std::string(a->name()), "Gridded2GridSpec");

    // the geographic domain
    EXPECT(a->outputBoundingBox() == util::BoundingBox(60, -10, 30, 40));
    EXPECT(make("{type: interpolate.grid2gridspec, grid: {grid: [10, 10]}}")->outputBoundingBox() ==
           util::BoundingBox(90, 0, -90, 360));

    EXPECT(a->json_str().find(R"("grid":{)") != std::string::npos);

    std::ostringstream print;
    print << *a;

    EXPECT(print.str().find("Gridded2GridSpec[gridspec=") == 0);
    EXPECT(print.str().find("interpolation=linear") != std::string::npos);
}


CASE("interpolate.grid2gridspec, comparison") {
    auto a = make("{type: interpolate.grid2gridspec, grid: {grid: O8}}");

    EXPECT(*a == *make("{type: interpolate.grid2gridspec, grid: O8}"));
    EXPECT(*a == *make("{type: interpolate.grid2gridspec, grid: {grid: O8}, interpolation: linear}"));

    EXPECT(*a != *make("{type: interpolate.grid2gridspec, grid: {grid: O16}}"));
    EXPECT(*a != *make("{type: interpolate.grid2gridspec, grid: {grid: O8}, interpolation: nn}"));
    EXPECT(*a != *make("{type: interpolate.grid2namedgrid, grid: O8}"));
}


CASE("interpolate.grid2gridspec, errors") {
    EXPECT_THROWS(make("interpolate.grid2gridspec"));                           // grid is required
    EXPECT_THROWS(make("{type: interpolate.grid2gridspec, grid: {grid: ?}}"));  // unknown grid
}


// projected grids (eckit::geo regular_xy), as GRIB lambert_azimuthal_equal_area, polar_stereographic and lambert
const std::string LAEA =
    R"({"projection":{"lat_0":52,"lon_0":10,"reverse":true,"type":"laea"},)"
    R"("x":[-1810621.83647687,2089378.16352313,100000],"y":[2283408.83487739,-1416591.16512261,-100000]})";

const std::string POLAR_STEREOGRAPHIC =
    R"({"projection":{"lat_0":90,"lat_ts":60,"reverse":true,"type":"stere"},)"
    R"("x":[-2252569.61519619,-352569.615196194,100000],"y":[-2252569.61519619,-4152569.61519619,-100000]})";

const std::string LAMBERT =
    R"({"projection":{"lat_0":50,"lat_1":50,"lon_0":10,"reverse":true,"type":"lcc"},)"
    R"("x":[-1707818.21687531,192181.783124688,100000],"y":[-887705.800790069,-2287705.80079007,-100000]})";


std::string spec(const std::string& grid, const std::string& options = "") {
    return "{type: interpolate.grid2gridspec, grid: " + grid + options + "}";
}


CASE("interpolate.grid2gridspec, projected grids") {
    struct test_t {
        std::string name;
        std::string grid;
        size_t numberOfPoints;
    };

    const std::vector<test_t> tests{
        test_t{"lambert_azimuthal_equal_area", LAEA, 40 * 38},
        test_t{"polar_stereographic", POLAR_STEREOGRAPHIC, 20 * 20},
        test_t{"lambert", LAMBERT, 20 * 15},
    };
    for (const auto& test : tests) {
        SECTION(test.name) {
            auto a   = make_checked(spec(test.grid));
            auto out = perform(*a, field("{grid: O16}", LATITUDE, 2));

            EXPECT_EQUAL(out.dimensions(), 2);
            EXPECT_EQUAL(out.values(0).size(), test.numberOfPoints);
            EXPECT_EQUAL(out.representation()->numberOfPoints(), test.numberOfPoints);
            EXPECT(out.representation()->uniqueName().find("RegularXY-") == 0);
            EXPECT(!out.hasMissing());

            // the grid corners are on the bounding box, contained only within (projection) rounding
            EXPECT(a->outputBoundingBox() == out.representation()->boundingBox());

            check_values(*a);
            check_values(*make_checked(spec(test.grid, ", interpolation: nn")), "{grid: O16}", 3.5);
        }
    }
}


CASE("interpolate.grid2gridspec, projected grids comparison") {
    auto a = make(spec(POLAR_STEREOGRAPHIC));

    EXPECT(*a == *make(spec(POLAR_STEREOGRAPHIC, ", interpolation: linear")));

    // the same spec, in a different order
    EXPECT(*a == *make(spec(R"({"y":[-2252569.61519619,-4152569.61519619,-100000],)"
                            R"("x":[-2252569.61519619,-352569.615196194,100000],)"
                            R"("projection":{"type":"stere","reverse":true,"lat_ts":60,"lat_0":90}})")));

    auto other = POLAR_STEREOGRAPHIC;
    other.replace(other.find("-4152569.61519619"), 17, "-4252569.61519619");

    EXPECT(*a != *make(spec(other)));

    EXPECT(*a != *make(spec(POLAR_STEREOGRAPHIC, ", interpolation: nn")));
    EXPECT(*a != *make(spec(LAEA)));
    EXPECT(*a != *make("{type: interpolate.grid2namedgrid, grid: O16}"));
}


CASE("interpolate.grid2gridspec, projected grids errors") {
    EXPECT_THROWS(make(spec(R"({"projection":{"type":"unknown"},"x":[0,1,1],"y":[0,1,1]})")));  // unknown projection
    EXPECT_THROWS(make(spec(R"({"projection":{"type":"laea","lat_0":52},"x":[0,1,1]})")));      // y missing
}


}  // namespace mir::action::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
