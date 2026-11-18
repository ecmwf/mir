// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <string>

#include "mir/util/BoundingBox.h"

#include "transform.h"


namespace mir::action::tests {


CASE("transform.sh-scalar-to-rotated-regular-ll") {
    const std::string spec = "{type: transform.sh-scalar-to-rotated-regular-ll, grid: [30, 30], rotation: [-40, 22]}";

    // values at the geographic points, as the grid is rotated
    SECTION("rotated") {
        check_scalar(spec, 7 * 12);
        check_representation(spec, "{grid: [30, 30], rotation: [-40, 22]}");
    }

    SECTION("rotated, multiple fields") {
        check_scalar(spec, 7 * 12, 2);
    }

    SECTION("area sets the grid reference, the transform is global") {
        auto out = perform(*make_checked("{type: transform.sh-scalar-to-rotated-regular-ll, grid: [5, 5], "
                                         "area: [62.5, -7.5, 32.5, 42.5], rotation: [-40, 22]}"),
                           sh(coefficients()));
        EXPECT_EQUAL(out.representation()->boundingBox(), util::BoundingBox(87.5, 2.5, -87.5, 357.5));
        EXPECT_EQUAL(out.values(0).size(), 36 * 72);
    }

    SECTION("cropping") {
        auto out = perform(*make_checked("{type: transform.sh-scalar-to-rotated-regular-ll, grid: [5, 5], "
                                         "cropping: [60, -10, 30, 40], rotation: [-40, 22]}"),
                           sh(coefficients({{index(0, 1), 1.}})));
        EXPECT_EQUAL(out.values(0).size(), 7 * 11);

        for_each(out, 0, [](double v, double lat, double) { EXPECT(same(v, P10(lat))); });
    }
}


CASE("transform.sh-vod-to-uv-rotated-regular-ll") {
    // winds relative to the geographic (not the rotated) grid
    const std::string spec = "{type: transform.sh-vod-to-uv-rotated-regular-ll, grid: [30, 30], rotation: [-40, 22]}";
    check_vod_to_uv(spec, 7 * 12);
    check_vod_to_uv(spec, 7 * 12, 2);
}


CASE("transform.sh-*-to-rotated-regular-ll, properties") {
    auto a = make("{type: transform.sh-scalar-to-rotated-regular-ll, grid: [30, 30], rotation: [-40, 22]}");
    auto b = make("{type: transform.sh-vod-to-uv-rotated-regular-ll, grid: [30, 30], rotation: [-40, 22]}");

    EXPECT_EQUAL(std::string(a->name()), "ShToRotatedRegularLL");
    EXPECT(contains(str(*a), "ShToRotatedRegularLL[type=local") && contains(str(*a), ",rotation="));
    EXPECT(contains(str(*b), "invtrans=<vod2uv>"));
    EXPECT(contains(a->json_str(false), R"("grid":[30,30],"area":[90,0,-90,360],"rotation":[-40,22])"));

    std::string name;

    EXPECT(!a->getGriddedTargetName(name));

    EXPECT(*a == *make("{type: transform.sh-scalar-to-rotated-regular-ll, grid: 30/30, rotation: -40/22}"));
    EXPECT(*a != *b);
    EXPECT(*a != *make("{type: transform.sh-scalar-to-rotated-regular-ll, grid: [20, 20], rotation: [-40, 22]}"));
    EXPECT(*a != *make("{type: transform.sh-scalar-to-rotated-regular-ll, grid: [30, 30], rotation: [-40, 20]}"));
    EXPECT(*a != *make("{type: transform.sh-scalar-to-regular-ll, grid: [30, 30]}"));
}


CASE("transform.sh-*-to-rotated-regular-ll, errors") {
    EXPECT_THROWS(make("{type: transform.sh-scalar-to-rotated-regular-ll, rotation: [-40, 22]}"));
    EXPECT_THROWS(make("{type: transform.sh-vod-to-uv-rotated-regular-ll, grid: [30, 30]}"));
    EXPECT_THROWS(make("{type: transform.sh-scalar-to-rotated-regular-ll, grid: [30], rotation: [-40, 22]}"));
    EXPECT_THROWS(make("{type: transform.sh-scalar-to-rotated-regular-ll, grid: [30, 30], rotation: [-40]}"));
    EXPECT_THROWS(perform("{type: transform.sh-scalar-to-rotated-regular-ll, grid: [30, 30], rotation: [-40, 22]}",
                          field("{grid: [30, 30]}")));
}


}  // namespace mir::action::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
