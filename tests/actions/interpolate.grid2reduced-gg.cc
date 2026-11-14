// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <sstream>
#include <vector>

#include "interpolate.h"


namespace mir::tests::actions {


CASE("interpolate.grid2reduced-gg") {
    struct test_t {
        size_t N;
        size_t numberOfPoints;
    };

    const std::vector<test_t> tests{test_t{32, 6114}};
    for (const auto& test : tests) {
        const auto N = std::to_string(test.N);

        SECTION("reduced: " + N) {
            auto a = make_checked("{type: interpolate.grid2reduced-gg, reduced: " + N + "}");

            std::string name;

            EXPECT(a->getGriddedTargetName(name));
            EXPECT_EQUAL(name, "N" + N);

            auto in  = field("{grid: O16}", LATITUDE, 2);
            auto out = perform(*a, in);

            EXPECT_EQUAL(out.dimensions(), 2);
            EXPECT_EQUAL(out.values(0).size(), test.numberOfPoints);
            EXPECT_EQUAL(out.representation()->numberOfPoints(), test.numberOfPoints);
            EXPECT(!out.hasMissing());

            // the same as the named grid
            auto named = perform("{type: interpolate.grid2namedgrid, grid: N" + N + "}", in);

            EXPECT(out.representation()->sameAs(*named.representation()));
            EXPECT(out.values(0) == named.values(0));
        }
    }
}


CASE("interpolate.grid2reduced-gg, values") {
    check_values(*make_checked("{type: interpolate.grid2reduced-gg, reduced: 32}"));
    check_values(*make_checked("{type: interpolate.grid2reduced-gg, reduced: 32, interpolation: nn}"), "{grid: O16}",
                 3.5);
}


CASE("interpolate.grid2reduced-gg, cropping") {
    auto a = make_mutable("{type: interpolate.grid2reduced-gg, reduced: 32}");

    EXPECT(!a->canCrop());

    EXPECT(a->mergeWithNext(*make("{type: crop.area, area: [60, -10, 30, 40]}")));
    EXPECT(a->canCrop());
    EXPECT(a->outputBoundingBox() == util::BoundingBox(60, -10, 30, 40));

    auto out = perform(*a, field("{grid: O16}", LATITUDE));

    EXPECT(0 < out.values(0).size() && out.values(0).size() < 6114);
    EXPECT(contained(out, a->outputBoundingBox()));
}


CASE("interpolate.grid2reduced-gg, properties") {
    auto a = make("{type: interpolate.grid2reduced-gg, reduced: 32}");

    EXPECT(a->isRegridAction());
    EXPECT(!a->canCrop());
    EXPECT_EQUAL(a->type(), "interpolate.grid2reduced-gg");
    EXPECT_EQUAL(std::string(a->name()), "Gridded2ReducedGG");
    EXPECT(a->outputBoundingBox() == util::BoundingBox(90, 0, -90, 360));
    EXPECT(a->json_str().find(R"("reduced":32)") != std::string::npos);

    std::ostringstream print;
    print << *a;

    EXPECT(print.str().find("Gridded2ReducedGG[N=32,interpolation=linear") == 0);
}


CASE("interpolate.grid2reduced-gg, comparison") {
    auto a = make("{type: interpolate.grid2reduced-gg, reduced: 32}");

    EXPECT(*a == *make("{type: interpolate.grid2reduced-gg, reduced: 32, interpolation: linear}"));

    EXPECT(*a != *make("{type: interpolate.grid2reduced-gg, reduced: 48}"));
    EXPECT(*a != *make("{type: interpolate.grid2reduced-gg, reduced: 32, interpolation: nn}"));
    EXPECT(*a != *make("{type: interpolate.grid2namedgrid, grid: N32}"));
    EXPECT(*a != *make("{type: interpolate.grid2rotated-reduced-gg, reduced: 32, rotation: [-40, 22]}"));
    EXPECT(*a != *make("{type: interpolate.grid2octahedral-gg, octahedral: 32}"));
}


CASE("interpolate.grid2reduced-gg, errors") {
    EXPECT_THROWS(make("interpolate.grid2reduced-gg"));  // reduced is required

    // N8 has no (classic) number of points per latitude
    EXPECT_THROWS(perform("{type: interpolate.grid2reduced-gg, reduced: 8}", field("{grid: O16}")));
}


}  // namespace mir::tests::actions


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
