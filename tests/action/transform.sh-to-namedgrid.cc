// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <string>
#include <vector>

#include "transform.h"


namespace mir::action::tests {


CASE("transform.sh-scalar-to-namedgrid") {
    const std::vector<std::string> grids{"O16", "N16", "F16"};
    for (const std::string& grid : grids) {
        SECTION(grid) {
            const auto spec = "{type: transform.sh-scalar-to-namedgrid, grid: " + grid + "}";
            check_scalar(spec, numberOfPoints(grid));
            check_scalar(spec, numberOfPoints(grid), 2);
            check_representation(spec, "{grid: " + grid + "}");
        }
    }

    SECTION("30/30") {
        check_scalar("{type: transform.sh-scalar-to-namedgrid, grid: 30/30}", 7 * 12);
    }
}


CASE("transform.sh-vod-to-uv-namedgrid") {
    check_vod_to_uv("{type: transform.sh-vod-to-uv-namedgrid, grid: O16}", 1600);
    check_vod_to_uv("{type: transform.sh-vod-to-uv-namedgrid, grid: F16}", 2048, 2);
}


CASE("transform.sh-*-to-namedgrid, plan compression") {
    const std::string spec = "{type: transform.sh-scalar-to-namedgrid, grid: O16}";

    SECTION("followed by an interpolation to the same grid: merge") {
        EXPECT(make_mutable(spec)->mergeWithNext(*make("{type: interpolate.grid2namedgrid, grid: O16}")));
        EXPECT(make_mutable(spec)->mergeWithNext(*make("{type: interpolate.grid2octahedral-gg, octahedral: 16}")));
    }

    SECTION("followed by an interpolation to another grid: no merge") {
        auto a = make_mutable(spec);

        EXPECT(!a->mergeWithNext(*make("{type: interpolate.grid2namedgrid, grid: N16}")));
        EXPECT(*a == *make(spec));
    }
}


CASE("transform.sh-*-to-namedgrid, properties") {
    auto a = make_checked("{type: transform.sh-scalar-to-namedgrid, grid: O16}");
    auto b = make_checked("{type: transform.sh-vod-to-uv-namedgrid, grid: O16}");

    EXPECT_EQUAL(std::string(a->name()), "ShToNamedGrid");
    EXPECT(contains(str(*a), "ShToNamedGrid[type=local") && contains(str(*a), ",grid=O16]"));
    EXPECT(contains(str(*b), "invtrans=<vod2uv>"));
    EXPECT(contains(a->json_str(false), R"("grid":"O16")"));

    // the grid name, canonical
    const std::vector<std::string> specs{"{type: transform.sh-scalar-to-namedgrid, grid: O16}",
                                         "{type: transform.sh-scalar-to-namedgrid, grid: o16}"};
    for (const auto& spec : specs) {
        std::string name;

        EXPECT(make(spec)->getGriddedTargetName(name));
        EXPECT_EQUAL(name, "O16");
        EXPECT(*a == *make(spec));
    }

    EXPECT(*a != *b);
    EXPECT(*a != *make("{type: transform.sh-scalar-to-namedgrid, grid: N16}"));
    EXPECT(*a != *make("{type: transform.sh-scalar-to-octahedral-gg, octahedral: 16}"));
    EXPECT(*a != *make("{type: transform.sh-scalar-to-namedgrid, grid: O16, atlas_trans_flt: true}"));
}


CASE("transform.sh-*-to-namedgrid, errors") {
    EXPECT_THROWS(make("transform.sh-scalar-to-namedgrid"));  // grid is required
    EXPECT_THROWS(make("{type: transform.sh-vod-to-uv-namedgrid, grid: unknown}"));
    EXPECT_THROWS(perform("{type: transform.sh-scalar-to-namedgrid, grid: O16}", field("{grid: O16}")));
}


}  // namespace mir::action::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
