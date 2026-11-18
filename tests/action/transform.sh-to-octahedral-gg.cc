// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <vector>

#include "transform.h"


namespace mir::action::tests {


CASE("transform.sh-scalar-to-octahedral-gg") {
    SECTION("O16") {
        check_scalar("{type: transform.sh-scalar-to-octahedral-gg, octahedral: 16}", 1600);
        check_representation("{type: transform.sh-scalar-to-octahedral-gg, octahedral: 16}", "{grid: O16}");
    }

    SECTION("O16, multiple fields") {
        check_scalar("{type: transform.sh-scalar-to-octahedral-gg, octahedral: 16}", 1600, 2);
    }
}


CASE("transform.sh-vod-to-uv-octahedral-gg") {
    check_vod_to_uv("{type: transform.sh-vod-to-uv-octahedral-gg, octahedral: 16}", 1600);
    check_vod_to_uv("{type: transform.sh-vod-to-uv-octahedral-gg, octahedral: 16}", 1600, 2);
}


CASE("transform.sh-*-to-octahedral-gg, plan compression") {
    const std::string spec = "{type: transform.sh-scalar-to-octahedral-gg, octahedral: 16}";

    SECTION("followed by an interpolation to the same grid: merge") {
        EXPECT(make_mutable(spec)->mergeWithNext(*make("{type: interpolate.grid2octahedral-gg, octahedral: 16}")));
        EXPECT(make_mutable(spec)->mergeWithNext(*make("{type: interpolate.grid2namedgrid, grid: O16}")));
    }

    SECTION("followed by an interpolation to another grid: no merge") {
        auto a = make_mutable(spec);

        EXPECT(!a->mergeWithNext(*make("{type: interpolate.grid2octahedral-gg, octahedral: 32}")));
        EXPECT(!a->mergeWithNext(*make("{type: interpolate.grid2reduced-gg, reduced: 16}")));
        EXPECT(*a == *make(spec));
    }

    SECTION("followed by cropping: merge") {
        auto a = make_mutable(spec);

        EXPECT(a->mergeWithNext(*make("{type: crop.area, area: [60, -10, 30, 40]}")));
        EXPECT(*a != *make(spec));

        auto out = perform(*a, sh(coefficients({{index(0, 1), 1.}})));

        EXPECT(0 < out.values(0).size() && out.values(0).size() < 1600);

        for_each(out, 0, [](double v, double lat, double lon) {
            EXPECT(30 <= lat && lat <= 60 && (lon <= 40 || 350 <= lon));
            EXPECT(same(v, P10(lat)));
        });
    }
}


CASE("transform.sh-*-to-octahedral-gg, properties") {
    auto a = make("{type: transform.sh-scalar-to-octahedral-gg, octahedral: 16}");
    auto b = make("{type: transform.sh-vod-to-uv-octahedral-gg, octahedral: 16}");

    EXPECT_EQUAL(std::string(a->name()), "ShToOctahedralGG");
    EXPECT(contains(str(*a), "ShToOctahedralGG[type=local") && contains(str(*a), ",N=16]"));
    EXPECT(contains(str(*b), "invtrans=<vod2uv>"));

    const std::vector<const action::Action*> actions{a.get(), b.get()};
    for (const auto* x : actions) {
        std::string name;

        EXPECT(x->getGriddedTargetName(name));
        EXPECT_EQUAL(name, "O16");
    }

    EXPECT(*a == *make("{type: transform.sh-scalar-to-octahedral-gg, octahedral: 16}"));
    EXPECT(*a != *b);
    EXPECT(*a != *make("{type: transform.sh-scalar-to-octahedral-gg, octahedral: 32}"));
    EXPECT(*a != *make("{type: transform.sh-scalar-to-octahedral-gg, octahedral: 16, atlas_trans_flt: true}"));
    EXPECT(*a != *make("{type: transform.sh-scalar-to-reduced-gg, reduced: 16}"));
    EXPECT(*a != *make("{type: transform.sh-scalar-to-namedgrid, grid: O16}"));
}


CASE("transform.sh-*-to-octahedral-gg, errors") {
    EXPECT_THROWS(make("transform.sh-scalar-to-octahedral-gg"));  // octahedral is required
    EXPECT_THROWS(make("transform.sh-vod-to-uv-octahedral-gg"));
    EXPECT_THROWS(perform("{type: transform.sh-scalar-to-octahedral-gg, octahedral: 16}", field("{grid: O16}")));
    EXPECT_THROWS(perform("{type: transform.sh-vod-to-uv-octahedral-gg, octahedral: 16}", sh(coefficients(), 1)));
}


}  // namespace mir::action::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
