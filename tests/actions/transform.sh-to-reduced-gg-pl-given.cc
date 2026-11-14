// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "transform.h"


namespace mir::tests::actions::transform {


// pl of O4
const std::string PL = "pl: [20, 24, 28, 32, 32, 28, 24, 20]";
constexpr size_t N   = 208;


CASE("transform.sh-scalar-to-reduced-gg-pl-given") {
    const std::string spec = "{type: transform.sh-scalar-to-reduced-gg-pl-given, " + PL + "}";

    SECTION("O4") {
        check_scalar(spec, N);
        check_representation(spec, "{grid: O4}");
    }

    SECTION("O4, multiple fields") {
        check_scalar(spec, N, 3);
    }

    SECTION("non-octahedral pl") {
        check_scalar("{type: transform.sh-scalar-to-reduced-gg-pl-given, pl: [6, 12, 12, 6]}", 36);
    }
}


CASE("transform.sh-vod-to-uv-reduced-gg-pl-given") {
    const std::string spec = "{type: transform.sh-vod-to-uv-reduced-gg-pl-given, " + PL + "}";
    check_vod_to_uv(spec, N);
    check_vod_to_uv(spec, N, 2);
}


CASE("transform.sh-*-to-reduced-gg-pl-given, plan compression") {
    const std::string spec = "{type: transform.sh-scalar-to-reduced-gg-pl-given, " + PL + "}";

    SECTION("followed by an interpolation to the same grid: merge") {
        EXPECT(make_mutable(spec)->mergeWithNext(*make("{type: interpolate.grid2reduced-gg-pl-given, " + PL + "}")));
    }

    SECTION("followed by cropping: merge") {
        auto a = make_mutable(spec);

        EXPECT(a->mergeWithNext(*make("{type: crop.area, area: [60, -10, 30, 40]}")));

        auto out = perform(*a, sh(coefficients({{index(0, 0), 2.}})));

        EXPECT(0 < out.values(0).size() && out.values(0).size() < N);

        for_each(out, 0, [](double v, double lat, double) {
            EXPECT(30 <= lat && lat <= 60);
            EXPECT(same(v, 2.));
        });
    }
}


CASE("transform.sh-*-to-reduced-gg-pl-given, properties") {
    auto a = make_checked("{type: transform.sh-scalar-to-reduced-gg-pl-given, " + PL + "}");
    auto b = make_checked("{type: transform.sh-vod-to-uv-reduced-gg-pl-given, " + PL + "}");

    EXPECT_EQUAL(std::string(a->name()), "ShToReducedGGPLGiven");
    EXPECT(contains(str(*a), "ShToReducedGGPLGiven[type=local") && contains(str(*a), ",pl=8]"));
    EXPECT(contains(str(*b), "invtrans=<vod2uv>"));
    EXPECT(contains(a->json_str(false), R"("pl":[20,24,28,32,32,28,24,20])"));

    std::string name;

    EXPECT(a->getGriddedTargetName(name));

    EXPECT(*a != *b);
    EXPECT(*a != *make("{type: transform.sh-scalar-to-reduced-gg-pl-given, pl: [20, 24, 28, 32, 32, 28, 24, 16]}"));
    EXPECT(*a != *make("{type: transform.sh-scalar-to-octahedral-gg, octahedral: 4}"));
    EXPECT(*a != *make("{type: transform.sh-scalar-to-reduced-gg-pl-given, atlas_trans_flt: true, " + PL + "}"));
}


CASE("transform.sh-*-to-reduced-gg-pl-given, errors") {
    EXPECT_THROWS(make("transform.sh-scalar-to-reduced-gg-pl-given"));  // pl is required
    EXPECT_THROWS(make("transform.sh-vod-to-uv-reduced-gg-pl-given"));
    EXPECT_THROWS(perform("{type: transform.sh-scalar-to-reduced-gg-pl-given, " + PL + "}", field("{grid: [10, 10]}")));
}


}  // namespace mir::tests::actions::transform


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
