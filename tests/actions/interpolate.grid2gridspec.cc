// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <sstream>
#include <vector>

#include "interpolate.h"


namespace mir::tests::actions {


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


}  // namespace mir::tests::actions


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
