// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <vector>

#include "action.h"


namespace mir::tests::actions {


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

            for (std::unique_ptr<repres::Iterator> it(out.representation()->iterator()); it->next();) {
                const auto lat = it->pointUnrotated().lat().value();
                const auto v   = out.values(1)[it->index()];

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


}  // namespace mir::tests::actions


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
