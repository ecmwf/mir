// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <sstream>
#include <vector>

#include "interpolate.h"


namespace mir::tests::actions {


CASE("interpolate.grid2namedgrid") {
    struct test_t {
        std::string grid;
        std::string name;  // canonical
        size_t numberOfPoints;
    };

    const std::vector<test_t> tests{
        test_t{"O8", "O8", 544},    test_t{"o8", "O8", 544},           test_t{"F8", "F8", 512},
        test_t{"N32", "N32", 6114}, test_t{"10/10", "10/10", 19 * 36},
    };
    for (const auto& test : tests) {
        SECTION(test.grid) {
            auto a = make_checked("{type: interpolate.grid2namedgrid, grid: " + test.grid + "}");

            std::string name;

            EXPECT(a->getGriddedTargetName(name));
            EXPECT_EQUAL(name, test.name);

            auto out = perform(*a, field("{grid: O16}", LATITUDE, 2));

            EXPECT_EQUAL(out.dimensions(), 2);
            EXPECT_EQUAL(out.values(0).size(), test.numberOfPoints);
            EXPECT_EQUAL(out.representation()->numberOfPoints(), test.numberOfPoints);
            EXPECT(!out.hasMissing());
            EXPECT(contained(out, a->outputBoundingBox()));
        }
    }
}


CASE("interpolate.grid2namedgrid, values") {
    const std::vector<std::string> grids{"O8", "F8", "N32"};
    for (const std::string& grid : grids) {
        SECTION(grid) {
            check_values(*make_checked("{type: interpolate.grid2namedgrid, grid: " + grid + "}"));
        }
    }
}


CASE("interpolate.grid2namedgrid, cropping") {
    auto a = make_mutable("{type: interpolate.grid2namedgrid, grid: O8}");

    EXPECT(!a->canCrop());

    EXPECT(a->mergeWithNext(*make("{type: crop.area, area: [60, -10, 30, 40]}")));
    EXPECT(a->canCrop());
    EXPECT(a->outputBoundingBox() == util::BoundingBox(60, -10, 30, 40));

    auto out = perform(*a, field("{grid: O16}", LATITUDE));

    EXPECT(0 < out.values(0).size() && out.values(0).size() < 544);
    EXPECT(contained(out, a->outputBoundingBox()));
}


CASE("interpolate.grid2namedgrid, properties") {
    auto a = make("{type: interpolate.grid2namedgrid, grid: o8}");

    EXPECT(a->isRegridAction());
    EXPECT(!a->canCrop());
    EXPECT_EQUAL(a->type(), "interpolate.grid2namedgrid");
    EXPECT_EQUAL(std::string(a->name()), "Gridded2NamedGrid");
    EXPECT(a->outputBoundingBox() == util::BoundingBox(90, 0, -90, 360));
    EXPECT(a->json_str().find(R"("grid":"O8")") != std::string::npos);

    std::ostringstream print;
    print << *a;

    EXPECT(print.str().find("Gridded2NamedGrid[grid=O8,interpolation=linear") == 0);
}


CASE("interpolate.grid2namedgrid, comparison") {
    auto a = make("{type: interpolate.grid2namedgrid, grid: O8}");

    EXPECT(*a == *make("{type: interpolate.grid2namedgrid, grid: o8}"));
    EXPECT(*a == *make("{type: interpolate.grid2namedgrid, grid: O8, interpolation: linear}"));

    EXPECT(*a != *make("{type: interpolate.grid2namedgrid, grid: O16}"));
    EXPECT(*a != *make("{type: interpolate.grid2namedgrid, grid: N8}"));
    EXPECT(*a != *make("{type: interpolate.grid2namedgrid, grid: O8, interpolation: nn}"));
    EXPECT(*a != *make("{type: interpolate.grid2octahedral-gg, octahedral: 8}"));
    EXPECT(*a != *make("{type: interpolate.grid2rotated-namedgrid, grid: O8, rotation: [-40, 22]}"));
}


CASE("interpolate.grid2namedgrid, errors") {
    EXPECT_THROWS(make("interpolate.grid2namedgrid"));                         // grid is required
    EXPECT_THROWS(make("{type: interpolate.grid2namedgrid, grid: unknown}"));  // unknown grid

    // N8 has no (classic) number of points per latitude
    EXPECT_THROWS(perform("{type: interpolate.grid2namedgrid, grid: N8}", field("{grid: O16}")));
}


}  // namespace mir::tests::actions


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
