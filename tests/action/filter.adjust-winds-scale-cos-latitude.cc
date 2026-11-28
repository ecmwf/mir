// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <cmath>
#include <vector>

#include "mir/util/Angles.h"

#include "action.h"
#include "winds.h"


namespace mir::action::tests {


CASE("filter.adjust-winds-scale-cos-latitude") {
    const std::vector<std::string> grids{"{grid: [10, 10]}", "{grid: O16}", "{grid: H8}"};
    for (const std::string& grid : grids) {
        SECTION(grid) {
            auto in  = field(grid, [](double, double lon) { return 1. + lon; }, 2);
            auto out = perform("filter.adjust-winds-scale-cos-latitude", in);

            EXPECT_EQUAL(out.dimensions(), 2);

            // scaled by 1/cos(latitude), zero at the poles
            for (size_t which = 0; which < 2; ++which) {
                const auto& points = out.representation()->canonicalGridUnrotated();
                for (auto it = points.cbegin(); it; ++it) {
                    const auto lat = std::get<PointLonLat>(*it).lat();
                    const auto in_ = in.values(which)[it.index()];
                    const auto v   = out.values(which)[it.index()];

                    EXPECT(std::abs(lat) == 90. ? v == 0.
                                                : same(v, in_ / std::cos(util::degree_to_radian(lat)), 1e-9 * v));
                }
            }
        }
    }
}


CASE("filter.adjust-winds-scale-cos-latitude, properties") {
    auto a = make("filter.adjust-winds-scale-cos-latitude");

    EXPECT_EQUAL(a->json_str(), R"({"type":"filter.adjust-winds-scale-cos-latitude"})");
    EXPECT(*a == *make_checked("{type: filter.adjust-winds-scale-cos-latitude}"));
    EXPECT(*a != *make("{type: filter.adjust-winds-directions, rotation: [-40, 22]}"));
}


CASE("filter.adjust-winds-scale-cos-latitude, errors") {
    // missing values are not supported
    auto in = field("{grid: [10, 10]}", [](double lat, double) { return lat > 50 ? 9999. : lat; }, 2, 9999.);

    EXPECT_THROWS(perform("filter.adjust-winds-scale-cos-latitude", in));
}


CASE("filter.adjust-winds-scale-cos-latitude, printing") {
    EXPECT_EQUAL(str(*make("filter.adjust-winds-scale-cos-latitude")), "AdjustWindsScaleCosLatitude[]");
}


}  // namespace mir::action::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
