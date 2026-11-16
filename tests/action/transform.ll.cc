// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <cmath>
#include <sstream>
#include <string>
#include <vector>

#include "mir/util/BoundingBox.h"

#include "transform.h"


namespace mir::action::tests {


CASE("transform.sh-scalar-to-regular-ll") {
    SECTION("global") {
        check_scalar("{type: transform.sh-scalar-to-regular-ll, grid: [30, 30]}", 7 * 12);
    }

    SECTION("global, multiple fields") {
        check_scalar("{type: transform.sh-scalar-to-regular-ll, grid: [30, 30]}", 7 * 12, 3);
    }

    SECTION("output representation") {
        auto out = perform("{type: transform.sh-scalar-to-regular-ll, grid: [5, 5]}", sh(coefficients()));

        EXPECT(out.representation()->isGlobal());
        EXPECT(out.representation()->boundingBox() == util::BoundingBox(90, 0, -90, 355));
        EXPECT_EQUAL(out.representation()->numberOfPoints(), 37 * 72);
    }

    SECTION("area sets the grid reference, the transform is global") {
        auto a =
            make_checked("{type: transform.sh-scalar-to-regular-ll, grid: [5, 5], area: [62.5, -7.5, 32.5, 42.5]}");
        auto out = perform(*a, sh(coefficients({{index(0, 0), 2.}})));

        EXPECT_EQUAL(out.representation()->boundingBox(), util::BoundingBox(87.5, 2.5, -87.5, 357.5));
        EXPECT_EQUAL(out.representation()->numberOfPoints(), 36 * 72);

        for_each(out, 0, [](double v, double, double) { EXPECT(same(v, 2.)); });
    }

    SECTION("lower truncation input") {
        auto out = perform("{type: transform.sh-scalar-to-regular-ll, grid: [30, 30]}",
                           sh(coefficients({{index(0, 1, 7), 1.}}, 7), 1, 7));
        for_each(out, 0, [](double v, double lat, double) { EXPECT(same(v, P10(lat))); });
    }
}


CASE("transform.sh-vod-to-uv-regular-ll") {
    SECTION("global") {
        check_vod_to_uv("{type: transform.sh-vod-to-uv-regular-ll, grid: [30, 30]}", 7 * 12);
    }

    SECTION("global, multiple field pairs") {
        check_vod_to_uv("{type: transform.sh-vod-to-uv-regular-ll, grid: [30, 30]}", 7 * 12, 2);
    }

    SECTION("u/v paramId, as given") {
        auto out =
            perform(*make("{type: transform.sh-vod-to-uv-regular-ll, grid: [30, 30], paramId.u: 1, paramId.v: 2}"),
                    vod(coefficients(), coefficients()));
        EXPECT_EQUAL(out.metadata(0).at("paramId"), 1);
        EXPECT_EQUAL(out.metadata(1).at("paramId"), 2);
    }
}


CASE("transform.sh-*-to-regular-ll, atlas/trans options") {
    const std::string spec = "{type: transform.sh-scalar-to-regular-ll, grid: [30, 30]";

    SECTION("defaults") {
        auto json = make(spec + "}")->json_str(false);

        EXPECT(contains(json, R"("atlas-trans-type":"local")"));
        EXPECT(contains(json, R"("atlas-trans-flt":false)"));
        EXPECT(contains(json, R"("invtrans":"scalar")"));
        EXPECT(contains(make("{type: transform.sh-vod-to-uv-regular-ll, grid: [30, 30]}")->json_str(false),
                        R"("invtrans":"vod2uv")"));
    }

    SECTION("type") {
        EXPECT(*make(spec + "}") == *make(spec + ", atlas_trans_type: local}"));
        EXPECT_THROWS(make(spec + ", atlas_trans_type: unknown}"));
    }

    SECTION("fast Legendre transform") {
        auto a = make_checked(spec + ", atlas_trans_flt: true}");

        EXPECT(contains(a->json_str(false), R"("atlas-trans-flt":true)"));
        EXPECT(*a != *make(spec + "}"));

        check_scalar(*a, 7 * 12);
    }

    SECTION("caching") {
        // a grid not transformed to before, so the coefficients are created in memory
        check_scalar(*make_checked("{type: transform.sh-scalar-to-regular-ll, grid: [20, 20], caching: false}"),
                     10 * 18);

        // the same coefficients again, from the in-memory cache
        check_scalar(*make_checked("{type: transform.sh-scalar-to-regular-ll, grid: [20, 20]}"), 10 * 18);
    }
}


CASE("transform.sh-*-to-regular-ll, cropping") {
    const util::BoundingBox bbox(60, -10, 30, 40);

    const std::vector<std::string> specs{
        "{type: transform.sh-scalar-to-regular-ll, grid: [5, 5], cropping: [60, -10, 30, 40]}",
        "{type: transform.sh-vod-to-uv-regular-ll, grid: [5, 5], cropping: [60, -10, 30, 40]}",
    };
    for (const auto& spec : specs) {
        SECTION(spec) {
            auto a = make_checked(spec);

            EXPECT(contains(a->json_str(false), R"("cropping":[60,-10,30,40])"));

            auto out = perform(*a, vod(coefficients({{index(0, 0), 2.}}), coefficients()));

            EXPECT(out.representation()->boundingBox() == bbox);
            EXPECT_EQUAL(out.representation()->numberOfPoints(), 7 * 11);

            for (std::unique_ptr<repres::Iterator> it(out.representation()->iterator()); it->next();) {
                EXPECT(bbox.contains(it->pointUnrotated()));
            }
        }
    }

    SECTION("cropped values") {
        auto out = perform("{type: transform.sh-scalar-to-regular-ll, grid: [5, 5], cropping: [60, -10, 30, 40]}",
                           sh(coefficients({{index(0, 1), 1.}})));
        EXPECT_EQUAL(out.values(0).size(), 7 * 11);

        for_each(out, 0, [](double v, double lat, double) { EXPECT(same(v, P10(lat))); });
    }

    EXPECT(*make("{type: transform.sh-scalar-to-regular-ll, grid: [5, 5], cropping: [60, -10, 30, 40]}") !=
           *make("{type: transform.sh-scalar-to-regular-ll, grid: [5, 5]}"));
    EXPECT_THROWS(make("{type: transform.sh-scalar-to-regular-ll, grid: [5, 5], cropping: [60, -10, 30]}"));
}


CASE("transform.sh-*-to-regular-ll, plan compression") {
    const std::string spec = "{type: transform.sh-scalar-to-regular-ll, grid: [5, 5]}";
    const std::string cropping(R"("cropping":[60,-10,30,40])");

    SECTION("followed by cropping: merge, transforming directly to the cropped grid") {
        auto a = make_mutable(spec);

        EXPECT(a->mergeWithNext(*make("{type: crop.area, area: [60, -10, 30, 40]}")));
        EXPECT(contains(a->json_str(false), cropping));
        EXPECT_EQUAL(perform(*a, sh(coefficients())).values(0).size(), 7 * 11);
    }

    SECTION("followed by an action that can crop: no merge, transforming to a containing grid") {
        auto a = make_mutable(spec);

        EXPECT(
            !a->mergeWithNext(*make("{type: interpolate.grid2regular-ll, grid: [1, 1], cropping: [58, -8, 32, 38]}")));
        EXPECT(contains(a->json_str(false), cropping));
    }

    SECTION("followed by an action that cannot crop: no merge") {
        auto a = make_mutable(spec);

        EXPECT(!a->mergeWithNext(*make("{type: interpolate.grid2regular-ll, grid: [1, 1]}")));
        EXPECT(!a->mergeWithNext(*make("{type: filter.globalise}")));
        EXPECT(*a == *make(spec));
    }

    SECTION("already cropped: no merge") {
        auto a = make_mutable("{type: transform.sh-scalar-to-regular-ll, grid: [5, 5], cropping: [60, -10, 30, 40]}");

        EXPECT(!a->mergeWithNext(*make("{type: crop.area, area: [50, 0, 40, 10]}")));
        EXPECT(contains(a->json_str(false), cropping));
    }
}


CASE("transform.sh-*-to-regular-ll, properties") {
    auto a = make("{type: transform.sh-scalar-to-regular-ll, grid: [5, 5]}");
    auto b = make("{type: transform.sh-vod-to-uv-regular-ll, grid: [5, 5]}");

    EXPECT_EQUAL(std::string(a->name()), "ShToRegularLL");
    EXPECT_EQUAL(a->type(), "transform.sh-scalar-to-regular-ll");
    EXPECT_EQUAL(b->type(), "transform.sh-vod-to-uv-regular-ll");

    std::ostringstream str;
    str << *a << *b;

    EXPECT(contains(str.str(), "ShToRegularLL[type=local,cropping=none"));
    EXPECT(contains(str.str(), "invtrans=<scalar>"));
    EXPECT(contains(str.str(), "invtrans=<vod2uv>"));

    std::string name;

    EXPECT(!a->getGriddedTargetName(name));
    EXPECT(!a->isRegridAction());
    EXPECT(!a->isCropAction());
    EXPECT(!a->canCrop());
    EXPECT(!a->isEndAction());
    EXPECT_THROWS(a->outputBoundingBox());

    EXPECT(*a == *make("{type: transform.sh-scalar-to-regular-ll, grid: 5/5}"));
    EXPECT(*a != *b);
    EXPECT(*a != *make("{type: transform.sh-scalar-to-regular-ll, grid: [10, 10]}"));
    EXPECT(*a != *make("{type: transform.sh-scalar-to-regular-ll, grid: [5, 5], area: [62.5, -7.5, 32.5, 42.5]}"));
    EXPECT(*a != *make("{type: transform.sh-scalar-to-regular-gg, regular: 16}"));
    EXPECT(*a != *make("{type: interpolate.grid2regular-ll, grid: [5, 5]}"));
}


CASE("transform.sh-*-to-regular-ll, errors") {
    EXPECT_THROWS(make("transform.sh-scalar-to-regular-ll"));  // grid is required
    EXPECT_THROWS(make("{type: transform.sh-scalar-to-regular-ll, grid: [1, 1, 1]}"));
    EXPECT_THROWS(make("{type: transform.sh-vod-to-uv-regular-ll, grid: [1]}"));

    const std::string scalar = "{type: transform.sh-scalar-to-regular-ll, grid: [30, 30]}";
    const std::string vod2uv = "{type: transform.sh-vod-to-uv-regular-ll, grid: [30, 30]}";

    // gridded input
    EXPECT_THROWS(perform(scalar, field("{grid: [10, 10]}")));
    EXPECT_THROWS(perform(vod2uv, field("{grid: [10, 10]}", F{}, 2)));

    // vorticity/divergence in pairs
    EXPECT_THROWS(perform(vod2uv, sh(coefficients(), 1)));
    EXPECT_THROWS(perform(vod2uv, sh(coefficients(), 3)));
}


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


CASE("transform.sh-vod-to-UV") {
    auto a = make_checked("transform.sh-vod-to-UV");

    SECTION("no vorticity/divergence, no wind") {
        auto out = perform(*a, vod(coefficients(), coefficients()));

        EXPECT_EQUAL(out.dimensions(), 2);
        EXPECT_EQUAL(out.representation()->truncation(), T);

        const std::vector<size_t> whichs{0, 1};
        for (size_t which : whichs) {
            EXPECT(out.values(which) == coefficients());
        }

        EXPECT_EQUAL(out.metadata(0).at("paramId"), 131);
        EXPECT_EQUAL(out.metadata(1).at("paramId"), 132);
    }

    SECTION("solid body rotation") {
        constexpr double vo = 1e-5;
        auto uv             = perform(*a, vod(coefficients({{index(0, 1), vo}}), coefficients()));

        EXPECT_EQUAL(uv.values(0).size(), coefficients().size());

        // U = u cos(latitude), V = v cos(latitude) (spectral) are the IFS wind components
        auto out = perform("{type: transform.sh-scalar-to-regular-ll, grid: [30, 30]}", uv);
        for_each(out, 0, [](double U, double lat, double) {
            EXPECT(same(U, solid_body_u(vo, lat) * std::cos(lat * M_PI / 180.), 1e-6));
        });
        for_each(out, 1, [](double V, double, double) { EXPECT(same(V, 0., 1e-6)); });
    }

    SECTION("lower truncation input") {
        auto out = perform(*a, vod(coefficients({}, 7), coefficients({}, 7), 1, 7));

        EXPECT_EQUAL(out.representation()->truncation(), 7);
        EXPECT(out.values(0) == coefficients({}, 7));
        EXPECT(out.values(1) == coefficients({}, 7));
    }

    SECTION("u/v paramId, as given") {
        auto out = perform(*make("{type: transform.sh-vod-to-UV, paramId.u: 1, paramId.v: 2}"),
                           vod(coefficients(), coefficients()));
        EXPECT_EQUAL(out.metadata(0).at("paramId"), 1);
        EXPECT_EQUAL(out.metadata(1).at("paramId"), 2);
    }
}


CASE("transform.sh-vod-to-UV, properties") {
    auto a = make("transform.sh-vod-to-UV");

    EXPECT_EQUAL(std::string(a->name()), "ShVodToUV");
    EXPECT_EQUAL(a->json_str(false), R"({"type":"transform.sh-vod-to-UV","atlas-trans-type":"local"})");

    std::ostringstream str;
    str << *a;

    EXPECT_EQUAL(str.str(), "ShVodToUV[type=local]");

    std::string name;

    EXPECT(!a->getGriddedTargetName(name));
    EXPECT(!a->isRegridAction());
    EXPECT(!a->canCrop());

    EXPECT(*a == *make("{type: transform.sh-vod-to-UV, atlas_trans_type: local}"));
    EXPECT(*a != *make("{type: transform.sh-vod-to-uv-regular-ll, grid: [30, 30]}"));
    EXPECT(*a != *make("{type: filter.adjust-winds-scale-cos-latitude}"));
}


CASE("transform.sh-vod-to-UV, errors") {
    EXPECT_THROWS(make("{type: transform.sh-vod-to-UV, atlas_trans_type: unknown}"));

    // vorticity/divergence, as one pair
    EXPECT_THROWS(perform("transform.sh-vod-to-UV", sh(coefficients(), 1)));
    EXPECT_THROWS(perform("transform.sh-vod-to-UV", vod(coefficients(), coefficients(), 2)));

    // vorticity/divergence of different truncations
    auto in = sh(coefficients(), 2);
    auto d  = coefficients({}, 7);
    in.update(d, 1);

    EXPECT_THROWS(perform("transform.sh-vod-to-UV", in));

    // gridded input
    EXPECT_THROWS(perform("transform.sh-vod-to-UV", field("{grid: [30, 30]}", F{}, 2)));
}


}  // namespace mir::action::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
