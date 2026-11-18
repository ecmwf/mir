// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <vector>

#include "action.h"


namespace mir::action::tests {


CASE("filter.globalise") {
    SECTION("regional, periodic west-east") {
        auto in = field("{grid: [10, 10], area: [60, 0, -30, 350]}", [](double lat, double) { return lat; }, 2);

        EXPECT_EQUAL(in.values(0).size(), 10 * 36);
        EXPECT(!in.representation()->isGlobal());

        auto out = perform("filter.globalise", in);

        EXPECT(out.representation()->isGlobal());
        EXPECT_EQUAL(out.dimensions(), 2);
        EXPECT_EQUAL(out.values(0).size(), 19 * 36);
        EXPECT(out.hasMissing());

        // new rows have missing values, old rows keep their values
        for (size_t which = 0; which < 2; ++which) {
            EXPECT_EQUAL(count_missing(out, which), 9 * 36);

            for (std::unique_ptr<repres::Iterator> it(out.representation()->iterator()); it->next();) {
                const auto lat = it->pointUnrotated().lat().value();
                const auto v   = out.values(which)[it->index()];

                EXPECT(lat > 60 || lat < -30 ? v == 9999. : same(v, lat));
            }
        }
    }

    const std::vector<std::string> grids{"{grid: [10, 10]}", "{grid: O16}"};
    for (const std::string& grid : grids) {
        SECTION("global " + grid + ": no effect") {
            auto in  = field(grid, [](double lat, double) { return lat; });
            auto out = perform("filter.globalise", in);

            EXPECT(out.representation()->sameAs(*in.representation()));
            EXPECT(out.values(0) == in.values(0));
            EXPECT(!out.hasMissing());
        }
    }
}


CASE("filter.globalise, properties") {
    std::unique_ptr<action::Action> a(action::ActionFactory::make_from_string("filter.globalise"));

    EXPECT(a->deleteWithNext(*make("{type: crop.area, area: [10, -10, -10, 10]}")));
    EXPECT(!a->deleteWithNext(*make("{type: mask.area, area: [10, -10, -10, 10]}")));
    EXPECT(!a->deleteWithNext(*make("filter.globalise")));

    EXPECT_EQUAL(a->json_str(), R"({"type":"filter.globalise"})");
    EXPECT(*a == *make_checked("{type: filter.globalise}"));
    EXPECT(*a != *make("filter.unstructured"));
}


CASE("filter.globalise, errors") {
    // regional, not periodic west-east
    EXPECT_THROWS(perform("filter.globalise", field("{grid: [10, 10], area: [60, 0, -30, 100]}")));
}


CASE("filter.globalise, printing") {
    EXPECT_EQUAL(str(*make("filter.globalise")), "GlobaliseFilter[]");
}


}  // namespace mir::action::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
