// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <sstream>
#include <vector>

#include "interpolate.h"


namespace mir::tests::actions {


// griddef files (in tests/data): a tour of the Azores, and a global set of points (including the poles)
const std::string TOUR   = "tour.pts";
const std::string GLOBAL = "gridType=unstructured_grid.pts";
const std::string AZORES = "[40, -32, 36, -24]";


CASE("interpolate.grid2griddef") {
    struct test_t {
        std::string griddef;
        size_t numberOfPoints;
        double latitude;  // of the first point
        double longitude;
    };

    const std::vector<test_t> tests{
        test_t{TOUR, 34, 37.742114, -25.664350},
        test_t{GLOBAL, 42, 90., 0.},
    };
    for (const auto& test : tests) {
        SECTION(test.griddef) {
            auto out = perform("{type: interpolate.grid2griddef, griddef: '" + test.griddef + "'}",
                               field("{grid: O16}", LATITUDE, 2));

            EXPECT_EQUAL(out.dimensions(), 2);
            EXPECT_EQUAL(out.values(0).size(), test.numberOfPoints);
            EXPECT_EQUAL(out.representation()->numberOfPoints(), test.numberOfPoints);
            EXPECT(out.representation()->uniqueName().find("unstructured-" + std::to_string(test.numberOfPoints)) == 0);

            std::unique_ptr<repres::Iterator> it(out.representation()->iterator());

            EXPECT(it->next());
            EXPECT(same(it->pointUnrotated().lat().value(), test.latitude));
            EXPECT(same(it->pointUnrotated().lon().value(), test.longitude));
        }
    }
}


CASE("interpolate.grid2griddef, values") {
    SECTION(TOUR) {
        check_values(*make_checked("{type: interpolate.grid2griddef, griddef: " + TOUR + "}"));
    }

    SECTION(GLOBAL) {
        // the poles are outside of the O16 latitudes
        check_values(*make_checked("{type: interpolate.grid2griddef, griddef: '" + GLOBAL + "'}"));
    }

    SECTION("nearest neighbour") {
        check_values(*make_checked("{type: interpolate.grid2griddef, griddef: " + TOUR + ", interpolation: nn}"),
                     "{grid: O16}", 3.);
    }
}


CASE("interpolate.grid2griddef, missing values") {
    auto out = perform("{type: interpolate.grid2griddef, griddef: '" + GLOBAL + "'}",
                       field("{grid: O16}", MISSING_NORTH, 1, 9999.));

    EXPECT(out.hasMissing());
    EXPECT_EQUAL(count_missing(out), 12);  // points at 60N and 90N
}


CASE("interpolate.grid2griddef, cropping") {
    auto a = make_mutable("{type: interpolate.grid2griddef, griddef: " + TOUR + "}");

    EXPECT(!a->canCrop());
    EXPECT(a->outputBoundingBox() == util::BoundingBox(90, 0, -90, 360));

    EXPECT(a->mergeWithNext(*make("{type: crop.area, area: " + AZORES + "}")));
    EXPECT(a->canCrop());
    EXPECT(a->outputBoundingBox() == util::BoundingBox(40, -32, 36, -24));

    auto out = perform(*a, field("{grid: O16}", LATITUDE));

    EXPECT_EQUAL(out.values(0).size(), 34);
    EXPECT(contained(out, a->outputBoundingBox()));
}


CASE("interpolate.grid2griddef, properties") {
    auto a = make_checked("{type: interpolate.grid2griddef, griddef: " + TOUR + "}");

    EXPECT(a->isRegridAction());
    EXPECT(!a->canCrop());

    std::string name;

    EXPECT(!a->getGriddedTargetName(name));

    EXPECT_EQUAL(a->type(), "interpolate.grid2griddef");
    EXPECT_EQUAL(std::string(a->name()), "Gridded2GridDef");
    EXPECT(a->json_str().find(R"("griddef":"tour.pts")") != std::string::npos);

    std::ostringstream print;
    print << *a;

    EXPECT(print.str().find("Gridded2GridDef[griddef=tour.pts,interpolation=linear") == 0);

    // the path is abbreviated
    std::ostringstream custom;
    make("{type: interpolate.grid2griddef, griddef: ../data/" + TOUR + "}")->custom(custom);

    EXPECT_EQUAL(custom.str(), "Gridded2GridDef[griddef=.../tour.pts]");
}


CASE("interpolate.grid2griddef, comparison") {
    auto a = make("{type: interpolate.grid2griddef, griddef: " + TOUR + "}");

    EXPECT(*a == *make("{type: interpolate.grid2griddef, griddef: " + TOUR + ", interpolation: linear}"));
    EXPECT(*a != *make("{type: interpolate.grid2griddef, griddef: '" + GLOBAL + "'}"));
    EXPECT(*a != *make("{type: interpolate.grid2griddef, griddef: " + TOUR + ", interpolation: nn}"));
    EXPECT(*a != *make("{type: interpolate.grid2namedgrid, grid: O16}"));
}


CASE("interpolate.grid2griddef, errors") {
    EXPECT_THROWS(make("interpolate.grid2griddef"));  // griddef is required

    // the griddef is read on use
    auto in = field("{grid: O16}");

    EXPECT_THROWS(perform("{type: interpolate.grid2griddef, griddef: does-not-exist.pts}", in));
    EXPECT_THROWS(perform("{type: interpolate.grid2griddef, griddef: /dev/null}", in));
}


}  // namespace mir::tests::actions


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
