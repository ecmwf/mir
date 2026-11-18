// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "transform.h"


namespace mir::action::tests {


CASE("transform.sh-scalar-to-rotated-reduced-gg") {
    const std::string spec = "{type: transform.sh-scalar-to-rotated-reduced-gg, reduced: 16, rotation: [-40, 22]}";

    // values at the geographic points, as the grid is rotated
    SECTION("N16, rotated") {
        check_scalar(spec, numberOfPoints("N16"));
        check_representation(spec, "{grid: N16, rotation: [-40, 22]}");
    }

    SECTION("N16, rotated, multiple fields") {
        check_scalar(spec, numberOfPoints("N16"), 2);
    }
}


CASE("transform.sh-vod-to-uv-rotated-reduced-gg") {
    // winds relative to the geographic (not the rotated) grid
    const std::string spec = "{type: transform.sh-vod-to-uv-rotated-reduced-gg, reduced: 16, rotation: [-40, 22]}";
    check_vod_to_uv(spec, numberOfPoints("N16"));
    check_vod_to_uv(spec, numberOfPoints("N16"), 2);
}


CASE("transform.sh-*-to-rotated-reduced-gg, properties") {
    auto a = make("{type: transform.sh-scalar-to-rotated-reduced-gg, reduced: 16, rotation: [-40, 22]}");
    auto b = make("{type: transform.sh-vod-to-uv-rotated-reduced-gg, reduced: 16, rotation: [-40, 22]}");

    EXPECT_EQUAL(std::string(a->name()), "ShToRotatedReducedGG");
    EXPECT(contains(str(*a), "ShToRotatedReducedGG[type=local") && contains(str(*a), ",N=16,rotation="));
    EXPECT(contains(str(*b), "invtrans=<vod2uv>"));
    EXPECT(contains(a->json_str(false), R"("reduced":16,"rotation":[-40,22])"));

    std::string name;

    EXPECT(!a->getGriddedTargetName(name));

    EXPECT(*a == *make("{type: transform.sh-scalar-to-rotated-reduced-gg, reduced: 16, rotation: -40/22}"));
    EXPECT(*a != *b);
    EXPECT(*a != *make("{type: transform.sh-scalar-to-rotated-reduced-gg, reduced: 32, rotation: [-40, 22]}"));
    EXPECT(*a != *make("{type: transform.sh-scalar-to-rotated-reduced-gg, reduced: 16, rotation: [-40, 20]}"));
    EXPECT(*a != *make("{type: transform.sh-scalar-to-reduced-gg, reduced: 16}"));
}


CASE("transform.sh-*-to-rotated-reduced-gg, errors") {
    EXPECT_THROWS(make("{type: transform.sh-scalar-to-rotated-reduced-gg, rotation: [-40, 22]}"));
    EXPECT_THROWS(make("{type: transform.sh-vod-to-uv-rotated-reduced-gg, reduced: 16}"));
    EXPECT_THROWS(make("{type: transform.sh-scalar-to-rotated-reduced-gg, reduced: 16, rotation: [-40]}"));
    EXPECT_THROWS(perform("{type: transform.sh-scalar-to-rotated-reduced-gg, reduced: 16, rotation: [-40, 22]}",
                          field("{grid: N16}")));
}


}  // namespace mir::action::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
