// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <vector>

#include "transform.h"


namespace mir::tests::actions::transform {


CASE("transform.sh-scalar-to-rotated-namedgrid") {
    // values at the geographic points, as the grid is rotated
    const std::vector<std::string> grids{"O8", "N16", "F8"};
    for (const std::string& grid : grids) {
        SECTION(grid) {
            const auto spec =
                "{type: transform.sh-scalar-to-rotated-namedgrid, grid: " + grid + ", rotation: [-40, 22]}";
            const auto n = representation("{grid: " + grid + "}")->numberOfPoints();
            check_scalar(spec, n, grid == "O8" ? 2 : 1);
            check_representation(spec, "{grid: " + grid + ", rotation: [-40, 22]}");
        }
    }
}


CASE("transform.sh-vod-to-uv-rotated-namedgrid") {
    // winds relative to the geographic (not the rotated) grid
    check_vod_to_uv("{type: transform.sh-vod-to-uv-rotated-namedgrid, grid: O8, rotation: [-40, 22]}", 544);
    check_vod_to_uv("{type: transform.sh-vod-to-uv-rotated-namedgrid, grid: O8, rotation: [-40, 22]}", 544, 2);
}


CASE("transform.sh-*-to-rotated-namedgrid, properties") {
    auto a = make_checked("{type: transform.sh-scalar-to-rotated-namedgrid, grid: O16, rotation: [-40, 22]}");
    auto b = make_checked("{type: transform.sh-vod-to-uv-rotated-namedgrid, grid: O16, rotation: [-40, 22]}");

    EXPECT_EQUAL(std::string(a->name()), "ShToRotatedNamedGrid");
    EXPECT(contains(str(*a), "ShToRotatedNamedGrid[type=local") && contains(str(*a), ",grid=O16,rotation="));
    EXPECT(contains(str(*b), "invtrans=<vod2uv>"));
    EXPECT(contains(a->json_str(false), R"("grid":"O16","rotation":[-40,22])"));

    std::string name;

    EXPECT(!a->getGriddedTargetName(name));

    EXPECT(*a == *make("{type: transform.sh-scalar-to-rotated-namedgrid, grid: O16, rotation: -40/22}"));
    EXPECT(*a != *b);
    EXPECT(*a != *make("{type: transform.sh-scalar-to-rotated-namedgrid, grid: N16, rotation: [-40, 22]}"));
    EXPECT(*a != *make("{type: transform.sh-scalar-to-rotated-namedgrid, grid: O16, rotation: [-40, 20]}"));
    EXPECT(*a != *make("{type: transform.sh-scalar-to-namedgrid, grid: O16}"));
}


CASE("transform.sh-*-to-rotated-namedgrid, errors") {
    EXPECT_THROWS(make("{type: transform.sh-scalar-to-rotated-namedgrid, rotation: [-40, 22]}"));
    EXPECT_THROWS(make("{type: transform.sh-vod-to-uv-rotated-namedgrid, grid: O16}"));
    EXPECT_THROWS(make("{type: transform.sh-scalar-to-rotated-namedgrid, grid: O16, rotation: [-40, 22, 0]}"));
    EXPECT_THROWS(perform("{type: transform.sh-scalar-to-rotated-namedgrid, grid: O16, rotation: [-40, 22]}",
                          field("{grid: O16}")));
}


}  // namespace mir::tests::actions::transform


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
