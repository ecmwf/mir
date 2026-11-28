// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <cmath>
#include <sstream>
#include <string>
#include <vector>

#include "transform.h"


namespace mir::action::tests {


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
