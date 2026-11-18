// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <sstream>
#include <vector>

#include "interpolate.h"


namespace mir::action::tests {


CASE("interpolate.grid2reduced-gg-pl-given") {
    struct test_t {
        std::string pl;
        size_t numberOfPoints;
    };

    const std::vector<test_t> tests{
        test_t{"[20, 24, 24, 20]", 88},
        test_t{"[20, 24, 28, 32, 32, 28, 24, 20]", 208},
    };
    for (const auto& test : tests) {
        SECTION("pl: " + test.pl) {
            auto a = make_checked("{type: interpolate.grid2reduced-gg-pl-given, pl: " + test.pl + "}");

            std::string name;

            EXPECT(a->getGriddedTargetName(name));

            auto out = perform(*a, field("{grid: O16}", LATITUDE, 2));

            EXPECT_EQUAL(out.dimensions(), 2);
            EXPECT_EQUAL(out.values(0).size(), test.numberOfPoints);
            EXPECT_EQUAL(out.representation()->numberOfPoints(), test.numberOfPoints);
            EXPECT(!out.hasMissing());
        }
    }

    SECTION("the same as the octahedral grid") {
        // O2 has 20, 24, 24, 20 points per latitude
        auto in  = field("{grid: O16}", LATITUDE);
        auto out = perform("{type: interpolate.grid2reduced-gg-pl-given, pl: [20, 24, 24, 20]}", in);
        auto oct = perform("{type: interpolate.grid2octahedral-gg, octahedral: 2}", in);

        EXPECT_EQUAL(out.values(0).size(), oct.values(0).size());

        for (size_t i = 0; i < out.values(0).size(); ++i) {
            EXPECT(same(out.values(0)[i], oct.values(0)[i]));
        }
    }
}


CASE("interpolate.grid2reduced-gg-pl-given, values") {
    check_values(*make_checked("{type: interpolate.grid2reduced-gg-pl-given, pl: [20, 24, 28, 32, 32, 28, 24, 20]}"));
}


CASE("interpolate.grid2reduced-gg-pl-given, properties") {
    auto a = make("{type: interpolate.grid2reduced-gg-pl-given, pl: [20, 24, 24, 20]}");

    EXPECT(a->isRegridAction());
    EXPECT(!a->canCrop());
    EXPECT_EQUAL(a->type(), "interpolate.grid2reduced-gg-pl-given");
    EXPECT_EQUAL(std::string(a->name()), "Gridded2ReducedGGPLGiven");
    EXPECT(a->outputBoundingBox() == util::BoundingBox(90, 0, -90, 360));
    EXPECT(a->json_str().find(R"("pl":[20,24,24,20])") != std::string::npos);

    std::ostringstream print;
    print << *a;

    EXPECT(print.str().find("Gridded2ReducedGGPLGiven[pl=4,interpolation=linear") == 0);
}


CASE("interpolate.grid2reduced-gg-pl-given, comparison") {
    auto a = make("{type: interpolate.grid2reduced-gg-pl-given, pl: [20, 24, 24, 20]}");

    EXPECT(*a == *make("{type: interpolate.grid2reduced-gg-pl-given, pl: [20, 24, 24, 20], interpolation: linear}"));

    EXPECT(*a != *make("{type: interpolate.grid2reduced-gg-pl-given, pl: [20, 20, 20, 20]}"));
    EXPECT(*a != *make("{type: interpolate.grid2reduced-gg-pl-given, pl: [20, 24, 24, 20], interpolation: nn}"));
    EXPECT(*a != *make("{type: interpolate.grid2octahedral-gg, octahedral: 2}"));
    EXPECT(*a !=
           *make("{type: interpolate.grid2rotated-reduced-gg-pl-given, pl: [20, 24, 24, 20], rotation: [-40, 22]}"));
}


CASE("interpolate.grid2reduced-gg-pl-given, errors") {
    EXPECT_THROWS(make("interpolate.grid2reduced-gg-pl-given"));  // pl is required
}


}  // namespace mir::action::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
