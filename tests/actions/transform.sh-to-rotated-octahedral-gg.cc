// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "transform.h"


namespace mir::tests::actions::transform {


CASE("transform.sh-scalar-to-rotated-octahedral-gg") {
    const std::string spec = "{type: transform.sh-scalar-to-rotated-octahedral-gg, octahedral: 8, rotation: [-40, 22]}";

    // values at the geographic points, as the grid is rotated
    SECTION("O8, rotated") {
        check_scalar(spec, 544);
        check_representation(spec, "{grid: O8, rotation: [-40, 22]}");
    }

    SECTION("O8, rotated, multiple fields") {
        check_scalar(spec, 544, 2);
    }
}


CASE("transform.sh-vod-to-uv-rotated-octahedral-gg") {
    // winds relative to the geographic (not the rotated) grid
    const std::string spec = "{type: transform.sh-vod-to-uv-rotated-octahedral-gg, octahedral: 8, rotation: [-40, 22]}";
    check_vod_to_uv(spec, 544);
    check_vod_to_uv(spec, 544, 2);
}


CASE("transform.sh-*-to-rotated-octahedral-gg, properties") {
    auto a = make("{type: transform.sh-scalar-to-rotated-octahedral-gg, octahedral: 8, rotation: [-40, 22]}");
    auto b = make("{type: transform.sh-vod-to-uv-rotated-octahedral-gg, octahedral: 8, rotation: [-40, 22]}");

    EXPECT_EQUAL(std::string(a->name()), "ShToRotatedOctahedralGG");
    EXPECT(contains(str(*a), "ShToRotatedOctahedralGG[type=local") && contains(str(*a), ",N=8,rotation="));
    EXPECT(contains(str(*b), "invtrans=<vod2uv>"));
    EXPECT(contains(a->json_str(false), R"("octahedral":8,"rotation":[-40,22])"));

    std::string name;

    EXPECT(!a->getGriddedTargetName(name));

    EXPECT(*a == *make("{type: transform.sh-scalar-to-rotated-octahedral-gg, octahedral: 8, rotation: -40/22}"));
    EXPECT(*a != *b);
    EXPECT(*a != *make("{type: transform.sh-scalar-to-rotated-octahedral-gg, octahedral: 16, rotation: [-40, 22]}"));
    EXPECT(*a != *make("{type: transform.sh-scalar-to-rotated-octahedral-gg, octahedral: 8, rotation: [-40, 20]}"));
    EXPECT(*a != *make("{type: transform.sh-scalar-to-octahedral-gg, octahedral: 8}"));
}


CASE("transform.sh-*-to-rotated-octahedral-gg, errors") {
    EXPECT_THROWS(make("{type: transform.sh-scalar-to-rotated-octahedral-gg, rotation: [-40, 22]}"));
    EXPECT_THROWS(make("{type: transform.sh-vod-to-uv-rotated-octahedral-gg, octahedral: 8}"));
    EXPECT_THROWS(make("{type: transform.sh-scalar-to-rotated-octahedral-gg, octahedral: 8, rotation: [-40]}"));
    EXPECT_THROWS(perform("{type: transform.sh-scalar-to-rotated-octahedral-gg, octahedral: 8, rotation: [-40, 22]}",
                          field("{grid: O8}")));
}


}  // namespace mir::tests::actions::transform


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
