// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <vector>

#include "mir/util/BoundingBox.h"

#include "action.h"


namespace mir::action::tests {


// first, so the area is not masked before in this process (masking is cached in memory)


CASE("mask.area, without caching") {
    auto in  = field("{grid: [1, 1]}", [](double lat, double) { return lat; }, 1, 9999.);
    auto out = perform("{type: mask.area, area: [50, -20, 20, 30], caching: false}", in);
    auto ref = perform("{type: mask.area, area: [50, -20, 20, 30]}", in);

    EXPECT_EQUAL(out.values(0).size(), in.values(0).size());
    EXPECT_EQUAL(out.values(0).size() - count_missing(out), 31 * 51);
    EXPECT(out.values(0) == ref.values(0));
}


CASE("mask.area") {
    const std::vector<std::string> grids{"{grid: [1, 1]}", "{grid: O32}", "{grid: H8}"};
    for (const std::string& grid : grids) {
        SECTION(grid) {
            auto in  = field(grid, [](double lat, double) { return lat; }, 2, 9999.);
            auto out = perform("{type: mask.area, area: [60, -10, 30, 40]}", in);

            // same points, missing values outside the area
            EXPECT_EQUAL(out.dimensions(), 2);
            EXPECT_EQUAL(out.values(0).size(), in.values(0).size());
            EXPECT(out.hasMissing());

            const auto& points = out.representation()->canonicalGridUnrotated();
            for (auto it = points.cbegin(); it; ++it) {
                const auto lat = std::get<PointLonLat>(*it).lat();
                const auto v   = out.values(1)[it.index()];

                EXPECT(v == 9999. || (30. - 1e-9 <= lat && lat <= 60. + 1e-9 && same(v, lat)));
            }
        }
    }
}


CASE("mask.area, properties") {
    auto a = make("{type: mask.area, area: [60, -10, 30, 40]}");

    EXPECT(!a->isCropAction());
    EXPECT(*a == *make("{type: mask.area, area: 60/-10/30/40}"));
    EXPECT(*a != *make("{type: mask.area, area: [60, -10, 30, 41]}"));
}


CASE("mask.area, errors") {
    EXPECT_THROWS(make("mask.area"));  // area is required
    EXPECT_THROWS(perform("{type: mask.area, area: [0.6, 0.2, 0.4, 0.8]}", field("{grid: [1, 1]}")));
}


CASE("mask.area, printing") {
    auto mask = make("{type: mask.area, area: [10, -10, -10, 10]}");

    EXPECT(str(*mask).rfind("AreaMasker[bbox=", 0) == 0);

    // masking keeps all points, its output bounding box is the area
    EXPECT(mask->outputBoundingBox() == util::BoundingBox(10, -10, -10, 10));
    EXPECT(!mask->canCrop());
}


}  // namespace mir::action::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
