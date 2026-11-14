// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <sstream>
#include <vector>

#include "interpolate.h"


namespace mir::tests::actions {


CASE("interpolate.grid2regular-gg") {
    struct test_t {
        size_t N;
        size_t numberOfPoints;
    };

    const std::vector<test_t> tests{test_t{8, 512}, test_t{16, 2048}};
    for (const auto& test : tests) {
        const auto N = std::to_string(test.N);

        SECTION("regular: " + N) {
            auto a = make_checked("{type: interpolate.grid2regular-gg, regular: " + N + "}");

            std::string name;

            EXPECT(a->getGriddedTargetName(name));
            EXPECT_EQUAL(name, "F" + N);

            auto in  = field("{grid: O16}", LATITUDE, 2);
            auto out = perform(*a, in);

            EXPECT_EQUAL(out.dimensions(), 2);
            EXPECT_EQUAL(out.values(0).size(), test.numberOfPoints);
            EXPECT_EQUAL(out.representation()->numberOfPoints(), test.numberOfPoints);
            EXPECT(!out.hasMissing());

            // the same as the named grid
            auto named = perform("{type: interpolate.grid2namedgrid, grid: F" + N + "}", in);

            EXPECT(out.representation()->sameAs(*named.representation()));
            EXPECT(out.values(0) == named.values(0));
        }
    }
}


CASE("interpolate.grid2regular-gg, values") {
    check_values(*make_checked("{type: interpolate.grid2regular-gg, regular: 8}"));
    check_values(*make_checked("{type: interpolate.grid2regular-gg, regular: 8, interpolation: nn}"), "{grid: O16}",
                 3.5);
}


CASE("interpolate.grid2regular-gg, cropping") {
    auto a = make_mutable("{type: interpolate.grid2regular-gg, regular: 8}");

    EXPECT(!a->canCrop());

    EXPECT(a->mergeWithNext(*make("{type: crop.area, area: [60, -10, 30, 40]}")));
    EXPECT(a->canCrop());
    EXPECT(a->outputBoundingBox() == util::BoundingBox(60, -10, 30, 40));

    auto out = perform(*a, field("{grid: O16}", LATITUDE));

    EXPECT(0 < out.values(0).size() && out.values(0).size() < 512);
    EXPECT(contained(out, a->outputBoundingBox()));
}


CASE("interpolate.grid2regular-gg, properties") {
    auto a = make("{type: interpolate.grid2regular-gg, regular: 8}");

    EXPECT(a->isRegridAction());
    EXPECT(!a->canCrop());
    EXPECT_EQUAL(a->type(), "interpolate.grid2regular-gg");
    EXPECT_EQUAL(std::string(a->name()), "Gridded2RegularGG");
    EXPECT(a->outputBoundingBox() == util::BoundingBox(90, 0, -90, 360));
    EXPECT(a->json_str().find(R"("regular":8)") != std::string::npos);

    std::ostringstream print;
    print << *a;

    EXPECT(print.str().find("Gridded2RegularGG[N=8,interpolation=linear") == 0);
}


CASE("interpolate.grid2regular-gg, comparison") {
    auto a = make("{type: interpolate.grid2regular-gg, regular: 8}");

    EXPECT(*a == *make("{type: interpolate.grid2regular-gg, regular: 8, interpolation: linear}"));

    EXPECT(*a != *make("{type: interpolate.grid2regular-gg, regular: 16}"));
    EXPECT(*a != *make("{type: interpolate.grid2regular-gg, regular: 8, interpolation: nn}"));
    EXPECT(*a != *make("{type: interpolate.grid2namedgrid, grid: F8}"));
    EXPECT(*a != *make("{type: interpolate.grid2rotated-regular-gg, regular: 8, rotation: [-40, 22]}"));
    EXPECT(*a != *make("{type: interpolate.grid2octahedral-gg, octahedral: 8}"));
}


CASE("interpolate.grid2regular-gg, errors") {
    EXPECT_THROWS(make("interpolate.grid2regular-gg"));  // regular is required
    EXPECT_THROWS(make("{type: interpolate.grid2regular-gg, regular: 0}"));
}


}  // namespace mir::tests::actions


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
