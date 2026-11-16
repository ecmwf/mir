// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <vector>

#include "transform.h"


namespace mir::action::tests {


CASE("transform.sh-scalar-to-regular-gg") {
    SECTION("F16") {
        check_scalar("{type: transform.sh-scalar-to-regular-gg, regular: 16}", 2048);
        check_representation("{type: transform.sh-scalar-to-regular-gg, regular: 16}", "{grid: F16}");
    }

    SECTION("F16, multiple fields") {
        check_scalar("{type: transform.sh-scalar-to-regular-gg, regular: 16}", 2048, 2);
    }
}


CASE("transform.sh-vod-to-uv-regular-gg") {
    check_vod_to_uv("{type: transform.sh-vod-to-uv-regular-gg, regular: 16}", 2048);
    check_vod_to_uv("{type: transform.sh-vod-to-uv-regular-gg, regular: 16}", 2048, 2);
}


CASE("transform.sh-*-to-regular-gg, plan compression") {
    const std::string spec = "{type: transform.sh-scalar-to-regular-gg, regular: 16}";

    SECTION("followed by an interpolation to the same grid: merge") {
        EXPECT(make_mutable(spec)->mergeWithNext(*make("{type: interpolate.grid2regular-gg, regular: 16}")));
        EXPECT(make_mutable(spec)->mergeWithNext(*make("{type: interpolate.grid2namedgrid, grid: F16}")));
    }

    SECTION("followed by an interpolation to another grid: no merge") {
        auto a = make_mutable(spec);

        EXPECT(!a->mergeWithNext(*make("{type: interpolate.grid2regular-gg, regular: 32}")));
        EXPECT(!a->mergeWithNext(*make("{type: interpolate.grid2octahedral-gg, octahedral: 16}")));
        EXPECT(*a == *make(spec));
    }

    SECTION("followed by cropping: merge") {
        auto a = make_mutable(spec);

        EXPECT(a->mergeWithNext(*make("{type: crop.area, area: [60, -10, 30, 40]}")));
        EXPECT(*a != *make(spec));

        auto out = perform(*a, sh(coefficients({{index(0, 1), 1.}})));

        EXPECT(0 < out.values(0).size() && out.values(0).size() < 2048);

        for_each(out, 0, [](double v, double lat, double lon) {
            EXPECT(30 <= lat && lat <= 60 && (lon <= 40 || 350 <= lon));
            EXPECT(same(v, P10(lat)));
        });
    }
}


CASE("transform.sh-*-to-regular-gg, properties") {
    auto a = make("{type: transform.sh-scalar-to-regular-gg, regular: 16}");
    auto b = make("{type: transform.sh-vod-to-uv-regular-gg, regular: 16}");

    EXPECT_EQUAL(std::string(a->name()), "ShToRegularGG");
    EXPECT(contains(str(*a), "ShToRegularGG[type=local") && contains(str(*a), ",N=16]"));
    EXPECT(contains(str(*b), "invtrans=<vod2uv>"));

    const std::vector<const action::Action*> actions{a.get(), b.get()};
    for (const auto* x : actions) {
        std::string name;

        EXPECT(x->getGriddedTargetName(name));
        EXPECT_EQUAL(name, "F16");
    }

    EXPECT(*a == *make("{type: transform.sh-scalar-to-regular-gg, regular: 16}"));
    EXPECT(*a != *b);
    EXPECT(*a != *make("{type: transform.sh-scalar-to-regular-gg, regular: 32}"));
    EXPECT(*a != *make("{type: transform.sh-scalar-to-regular-gg, regular: 16, atlas_trans_flt: true}"));
    EXPECT(*a != *make("{type: transform.sh-scalar-to-octahedral-gg, octahedral: 16}"));
    EXPECT(*a != *make("{type: transform.sh-scalar-to-namedgrid, grid: F16}"));
}


CASE("transform.sh-*-to-regular-gg, errors") {
    EXPECT_THROWS(make("transform.sh-scalar-to-regular-gg"));  // regular is required
    EXPECT_THROWS(make("transform.sh-vod-to-uv-regular-gg"));
    EXPECT_THROWS(perform("{type: transform.sh-scalar-to-regular-gg, regular: 16}", field("{grid: F16}")));
    EXPECT_THROWS(perform("{type: transform.sh-vod-to-uv-regular-gg, regular: 16}", sh(coefficients(), 1)));
}


CASE("transform.sh-scalar-to-rotated-regular-gg") {
    const std::string spec = "{type: transform.sh-scalar-to-rotated-regular-gg, regular: 8, rotation: [-40, 22]}";

    // values at the geographic points, as the grid is rotated
    SECTION("F8, rotated") {
        check_scalar(spec, 512);
        check_representation(spec, "{grid: F8, rotation: [-40, 22]}");
    }

    SECTION("F8, rotated, multiple fields") {
        check_scalar(spec, 512, 2);
    }
}


CASE("transform.sh-vod-to-uv-rotated-regular-gg") {
    // winds relative to the geographic (not the rotated) grid
    const std::string spec = "{type: transform.sh-vod-to-uv-rotated-regular-gg, regular: 8, rotation: [-40, 22]}";
    check_vod_to_uv(spec, 512);
    check_vod_to_uv(spec, 512, 2);
}


CASE("transform.sh-*-to-rotated-regular-gg, properties") {
    auto a = make("{type: transform.sh-scalar-to-rotated-regular-gg, regular: 8, rotation: [-40, 22]}");
    auto b = make("{type: transform.sh-vod-to-uv-rotated-regular-gg, regular: 8, rotation: [-40, 22]}");

    EXPECT_EQUAL(std::string(a->name()), "ShToRotatedRegularGG");
    EXPECT(contains(str(*a), "ShToRotatedRegularGG[type=local") && contains(str(*a), ",N=8,rotation="));
    EXPECT(contains(str(*b), "invtrans=<vod2uv>"));
    EXPECT(contains(a->json_str(false), R"("regular":8,"rotation":[-40,22])"));

    std::string name;

    EXPECT(!a->getGriddedTargetName(name));

    EXPECT(*a == *make("{type: transform.sh-scalar-to-rotated-regular-gg, regular: 8, rotation: -40/22}"));
    EXPECT(*a != *b);
    EXPECT(*a != *make("{type: transform.sh-scalar-to-rotated-regular-gg, regular: 16, rotation: [-40, 22]}"));
    EXPECT(*a != *make("{type: transform.sh-scalar-to-rotated-regular-gg, regular: 8, rotation: [-40, 20]}"));
    EXPECT(*a != *make("{type: transform.sh-scalar-to-regular-gg, regular: 8}"));
}


CASE("transform.sh-*-to-rotated-regular-gg, errors") {
    EXPECT_THROWS(make("{type: transform.sh-scalar-to-rotated-regular-gg, rotation: [-40, 22]}"));
    EXPECT_THROWS(make("{type: transform.sh-vod-to-uv-rotated-regular-gg, regular: 8}"));
    EXPECT_THROWS(make("{type: transform.sh-scalar-to-rotated-regular-gg, regular: 8, rotation: [-40]}"));
    EXPECT_THROWS(perform("{type: transform.sh-scalar-to-rotated-regular-gg, regular: 8, rotation: [-40, 22]}",
                          field("{grid: F8}")));
}


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


size_t N16() {
    return representation("{grid: N16}")->numberOfPoints();
}


CASE("transform.sh-scalar-to-reduced-gg") {
    SECTION("N16") {
        check_scalar("{type: transform.sh-scalar-to-reduced-gg, reduced: 16}", N16());
        check_representation("{type: transform.sh-scalar-to-reduced-gg, reduced: 16}", "{grid: N16}");
    }

    SECTION("N16, multiple fields") {
        check_scalar("{type: transform.sh-scalar-to-reduced-gg, reduced: 16}", N16(), 2);
    }
}


CASE("transform.sh-vod-to-uv-reduced-gg") {
    check_vod_to_uv("{type: transform.sh-vod-to-uv-reduced-gg, reduced: 16}", N16());
    check_vod_to_uv("{type: transform.sh-vod-to-uv-reduced-gg, reduced: 16}", N16(), 2);
}


CASE("transform.sh-*-to-reduced-gg, plan compression") {
    const std::string spec = "{type: transform.sh-scalar-to-reduced-gg, reduced: 16}";

    SECTION("followed by an interpolation to the same grid: merge") {
        EXPECT(make_mutable(spec)->mergeWithNext(*make("{type: interpolate.grid2reduced-gg, reduced: 16}")));
        EXPECT(make_mutable(spec)->mergeWithNext(*make("{type: interpolate.grid2namedgrid, grid: N16}")));
    }

    SECTION("followed by an interpolation to another grid: no merge") {
        auto a = make_mutable(spec);

        EXPECT(!a->mergeWithNext(*make("{type: interpolate.grid2reduced-gg, reduced: 32}")));
        EXPECT(!a->mergeWithNext(*make("{type: interpolate.grid2octahedral-gg, octahedral: 16}")));
        EXPECT(*a == *make(spec));
    }

    SECTION("followed by cropping: merge") {
        auto a = make_mutable(spec);

        EXPECT(a->mergeWithNext(*make("{type: crop.area, area: [60, -10, 30, 40]}")));
        EXPECT(*a != *make(spec));

        auto out = perform(*a, sh(coefficients({{index(0, 1), 1.}})));

        EXPECT(0 < out.values(0).size() && out.values(0).size() < N16());

        for_each(out, 0, [](double v, double lat, double lon) {
            EXPECT(30 <= lat && lat <= 60 && (lon <= 40 || 350 <= lon));
            EXPECT(same(v, P10(lat)));
        });
    }
}


CASE("transform.sh-*-to-reduced-gg, properties") {
    auto a = make("{type: transform.sh-scalar-to-reduced-gg, reduced: 16}");
    auto b = make("{type: transform.sh-vod-to-uv-reduced-gg, reduced: 16}");

    EXPECT_EQUAL(std::string(a->name()), "ShToReducedGG");
    EXPECT(contains(str(*a), "ShToReducedGG[type=local") && contains(str(*a), ",N=16]"));
    EXPECT(contains(str(*b), "invtrans=<vod2uv>"));

    const std::vector<const action::Action*> actions{a.get(), b.get()};
    for (const auto* x : actions) {
        std::string name;

        EXPECT(x->getGriddedTargetName(name));
        EXPECT_EQUAL(name, "N16");
    }

    EXPECT(*a == *make("{type: transform.sh-scalar-to-reduced-gg, reduced: 16}"));
    EXPECT(*a != *b);
    EXPECT(*a != *make("{type: transform.sh-scalar-to-reduced-gg, reduced: 32}"));
    EXPECT(*a != *make("{type: transform.sh-scalar-to-reduced-gg, reduced: 16, atlas_trans_flt: true}"));
    EXPECT(*a != *make("{type: transform.sh-scalar-to-octahedral-gg, octahedral: 16}"));
    EXPECT(*a != *make("{type: transform.sh-scalar-to-namedgrid, grid: N16}"));
}


CASE("transform.sh-*-to-reduced-gg, errors") {
    EXPECT_THROWS(make("transform.sh-scalar-to-reduced-gg"));  // reduced is required
    EXPECT_THROWS(make("transform.sh-vod-to-uv-reduced-gg"));
    EXPECT_THROWS(perform("{type: transform.sh-scalar-to-reduced-gg, reduced: 16}", field("{grid: N16}")));
    EXPECT_THROWS(perform("{type: transform.sh-vod-to-uv-reduced-gg, reduced: 16}", sh(coefficients(), 1)));
}


CASE("transform.sh-scalar-to-rotated-reduced-gg") {
    const std::string spec = "{type: transform.sh-scalar-to-rotated-reduced-gg, reduced: 16, rotation: [-40, 22]}";

    // values at the geographic points, as the grid is rotated
    SECTION("N16, rotated") {
        check_scalar(spec, N16());
        check_representation(spec, "{grid: N16, rotation: [-40, 22]}");
    }

    SECTION("N16, rotated, multiple fields") {
        check_scalar(spec, N16(), 2);
    }
}


CASE("transform.sh-vod-to-uv-rotated-reduced-gg") {
    // winds relative to the geographic (not the rotated) grid
    const std::string spec = "{type: transform.sh-vod-to-uv-rotated-reduced-gg, reduced: 16, rotation: [-40, 22]}";
    check_vod_to_uv(spec, N16());
    check_vod_to_uv(spec, N16(), 2);
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


// pl of O4
const std::string O4_PL    = "pl: [20, 24, 28, 32, 32, 28, 24, 20]";
constexpr size_t O4_POINTS = 208;


CASE("transform.sh-scalar-to-reduced-gg-pl-given") {
    const std::string spec = "{type: transform.sh-scalar-to-reduced-gg-pl-given, " + O4_PL + "}";

    SECTION("O4") {
        check_scalar(spec, O4_POINTS);
        check_representation(spec, "{grid: O4}");
    }

    SECTION("O4, multiple fields") {
        check_scalar(spec, O4_POINTS, 3);
    }

    SECTION("non-octahedral pl") {
        check_scalar("{type: transform.sh-scalar-to-reduced-gg-pl-given, pl: [6, 12, 12, 6]}", 36);
    }
}


CASE("transform.sh-vod-to-uv-reduced-gg-pl-given") {
    const std::string spec = "{type: transform.sh-vod-to-uv-reduced-gg-pl-given, " + O4_PL + "}";
    check_vod_to_uv(spec, O4_POINTS);
    check_vod_to_uv(spec, O4_POINTS, 2);
}


CASE("transform.sh-*-to-reduced-gg-pl-given, plan compression") {
    const std::string spec = "{type: transform.sh-scalar-to-reduced-gg-pl-given, " + O4_PL + "}";

    SECTION("followed by an interpolation to the same grid: merge") {
        EXPECT(make_mutable(spec)->mergeWithNext(*make("{type: interpolate.grid2reduced-gg-pl-given, " + O4_PL + "}")));
    }

    SECTION("followed by cropping: merge") {
        auto a = make_mutable(spec);

        EXPECT(a->mergeWithNext(*make("{type: crop.area, area: [60, -10, 30, 40]}")));

        auto out = perform(*a, sh(coefficients({{index(0, 0), 2.}})));

        EXPECT(0 < out.values(0).size() && out.values(0).size() < O4_POINTS);

        for_each(out, 0, [](double v, double lat, double) {
            EXPECT(30 <= lat && lat <= 60);
            EXPECT(same(v, 2.));
        });
    }
}


CASE("transform.sh-*-to-reduced-gg-pl-given, properties") {
    auto a = make_checked("{type: transform.sh-scalar-to-reduced-gg-pl-given, " + O4_PL + "}");
    auto b = make_checked("{type: transform.sh-vod-to-uv-reduced-gg-pl-given, " + O4_PL + "}");

    EXPECT_EQUAL(std::string(a->name()), "ShToReducedGGPLGiven");
    EXPECT(contains(str(*a), "ShToReducedGGPLGiven[type=local") && contains(str(*a), ",pl=8]"));
    EXPECT(contains(str(*b), "invtrans=<vod2uv>"));
    EXPECT(contains(a->json_str(false), R"("pl":[20,24,28,32,32,28,24,20])"));

    std::string name;

    EXPECT(a->getGriddedTargetName(name));

    EXPECT(*a != *b);
    EXPECT(*a != *make("{type: transform.sh-scalar-to-reduced-gg-pl-given, pl: [20, 24, 28, 32, 32, 28, 24, 16]}"));
    EXPECT(*a != *make("{type: transform.sh-scalar-to-octahedral-gg, octahedral: 4}"));
    EXPECT(*a != *make("{type: transform.sh-scalar-to-reduced-gg-pl-given, atlas_trans_flt: true, " + O4_PL + "}"));
}


CASE("transform.sh-*-to-reduced-gg-pl-given, errors") {
    EXPECT_THROWS(make("transform.sh-scalar-to-reduced-gg-pl-given"));  // pl is required
    EXPECT_THROWS(make("transform.sh-vod-to-uv-reduced-gg-pl-given"));
    EXPECT_THROWS(
        perform("{type: transform.sh-scalar-to-reduced-gg-pl-given, " + O4_PL + "}", field("{grid: [10, 10]}")));
}


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


}  // namespace mir::action::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
