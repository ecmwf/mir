// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "transform.h"


namespace mir::tests::actions::transform {


CASE("transform.sh-scalar-to-rotated-reduced-gg-pl-given") {
    const std::string spec =
        "{type: transform.sh-scalar-to-rotated-reduced-gg-pl-given, pl: [20, 24, 28, 32, 32, 28, 24, 20], rotation: "
        "[-40, 22]}";

    // values at the geographic points, as the grid is rotated
    SECTION("O4, rotated") {
        check_scalar(spec, 208);
        check_representation(spec, "{grid: O4, rotation: [-40, 22]}");
    }

    SECTION("O4, rotated, multiple fields") {
        check_scalar(spec, 208, 2);
    }
}


CASE("transform.sh-vod-to-uv-rotated-reduced-gg-pl-given") {
    // winds relative to the geographic (not the rotated) grid
    const std::string spec =
        "{type: transform.sh-vod-to-uv-rotated-reduced-gg-pl-given, pl: [20, 24, 28, 32, 32, 28, 24, 20], rotation: "
        "[-40, 22]}";
    check_vod_to_uv(spec, 208);
    check_vod_to_uv(spec, 208, 2);
}


CASE("transform.sh-*-to-rotated-reduced-gg-pl-given, properties") {
    auto a = make(
        "{type: transform.sh-scalar-to-rotated-reduced-gg-pl-given, pl: [20, 24, 28, 32, 32, 28, 24, 20], rotation: "
        "[-40, 22]}");
    auto b = make(
        "{type: transform.sh-vod-to-uv-rotated-reduced-gg-pl-given, pl: [20, 24, 28, 32, 32, 28, 24, 20], rotation: "
        "[-40, 22]}");

    EXPECT_EQUAL(std::string(a->name()), "ShToRotatedReducedGGPLGiven");
    EXPECT(contains(str(*a), "ShToRotatedReducedGGPLGiven[type=local") && contains(str(*a), ",pl=8,rotation="));
    EXPECT(contains(str(*b), "invtrans=<vod2uv>"));
    EXPECT(contains(a->json_str(false), R"("pl":[20,24,28,32,32,28,24,20],"rotation":[-40,22])"));

    std::string name;

    EXPECT(!a->getGriddedTargetName(name));

    EXPECT(*a == *make("{type: transform.sh-scalar-to-rotated-reduced-gg-pl-given, pl: [20, 24, 28, 32, 32, 28, 24, "
                       "20], rotation: -40/22}"));
    EXPECT(*a != *b);
    EXPECT(*a != *make("{type: transform.sh-scalar-to-rotated-reduced-gg-pl-given, pl: [20, 24, 28, 32, 32, 28, 24, "
                       "16], rotation: [-40, 22]}"));
    EXPECT(*a != *make("{type: transform.sh-scalar-to-rotated-reduced-gg-pl-given, pl: [20, 24, 28, 32, 32, 28, 24, "
                       "20], rotation: [-40, 20]}"));
    EXPECT(*a != *make("{type: transform.sh-scalar-to-reduced-gg-pl-given, pl: [20, 24, 28, 32, 32, 28, 24, 20]}"));
}


CASE("transform.sh-*-to-rotated-reduced-gg-pl-given, errors") {
    EXPECT_THROWS(make("{type: transform.sh-scalar-to-rotated-reduced-gg-pl-given, rotation: [-40, 22]}"));
    EXPECT_THROWS(
        make("{type: transform.sh-vod-to-uv-rotated-reduced-gg-pl-given, pl: [20, 24, 28, 32, 32, 28, 24, 20]}"));
    EXPECT_THROWS(
        make("{type: transform.sh-scalar-to-rotated-reduced-gg-pl-given, pl: [20, 24, 28, 32, 32, 28, 24, 20], "
             "rotation: [-40]}"));
    EXPECT_THROWS(
        perform("{type: transform.sh-scalar-to-rotated-reduced-gg-pl-given, pl: [20, 24, 28, 32, 32, 28, 24, 20], "
                "rotation: [-40, 22]}",
                field("{grid: O4}")));
}


}  // namespace mir::tests::actions::transform


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
