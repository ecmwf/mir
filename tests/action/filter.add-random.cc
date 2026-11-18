// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <cmath>
#include <cstdlib>
#include <numeric>

#include "action.h"


namespace mir::action::tests {


std::string add_random(const std::string& distribution) {
    return "{type: filter.add-random, add-random: '" + distribution + "'}";
}


CASE("filter.add-random") {
    auto in = field("{grid: [10, 10]}", nullptr, 2);

    SECTION("uniform-int-distribution") {
        auto out = perform(*make(add_random("uniform-int-distribution{a:1,b:3}")), in);

        EXPECT_EQUAL(out.dimensions(), 2);

        for (size_t which = 0; which < 2; ++which) {
            const auto& v = out.values(which);

            EXPECT_EQUAL(v.size(), in.values(which).size());

            size_t count[3]{0, 0, 0};

            for (const auto& x : v) {
                EXPECT(x == 1. || x == 2. || x == 3.);

                count[static_cast<size_t>(x) - 1]++;
            }

            // 684 samples: all values are drawn
            EXPECT(count[0] > 0 && count[1] > 0 && count[2] > 0);
        }
    }

    SECTION("normal-distribution") {
        auto out = perform(*make(add_random("normal-distribution{mean:42.,stddev:3.}")), in);

        // 684 samples: the mean standard error is about 0.11
        const auto& v   = out.values(0);
        const auto mean = std::accumulate(v.begin(), v.end(), 0.) / static_cast<double>(v.size());

        EXPECT(std::abs(mean - 42.) < 1.);
    }

    SECTION("bernoulli-distribution, default arguments") {
        auto out = perform(*make(add_random("bernoulli-distribution{}")), in);

        for (const auto& x : out.values(0)) {
            EXPECT(x == 0. || x == 1.);
        }
    }
}


CASE("filter.add-random, with missing values") {
    auto in  = field("{grid: [10, 10]}", [](double lat, double) { return lat > 50 ? 9999. : 0.; }, 1, 9999.);
    auto out = perform(*make(add_random("uniform-int-distribution{a:1,b:3}")), in);

    EXPECT(out.hasMissing());
    EXPECT_EQUAL(count_missing(out), count_missing(in));

    for (size_t i = 0; i < in.values(0).size(); ++i) {
        const auto x = out.values(0)[i];

        EXPECT(in.values(0)[i] == 9999. ? x == 9999. : (1. <= x && x <= 3.));
    }
}


CASE("filter.add-random, properties") {
    auto a = make(add_random("uniform-int-distribution{a:1,b:3}"));

    EXPECT_EQUAL(a->json_str(), R"({"type":"filter.add-random","add-random":"uniform-int-distribution{a:1,b:3}"})");

    auto b = make(a->json_str(false));

    EXPECT_EQUAL(b->json_str(false), a->json_str(false));
}


CASE("filter.add-random, errors") {
    EXPECT_THROWS(make("filter.add-random"));  // add-random is required
    EXPECT_THROWS(make(add_random("unknown-distribution{}")));
}


CASE("filter.add-random, printing") {
    EXPECT(str(*make(add_random("uniform-int-distribution{a:1,b:3}"))).rfind("AddRandomFilter[distribution=", 0) == 0);
}


CASE("filter.add-random, comparison") {
    // random values are never the same, so neither are the actions
    auto a = make(add_random("uniform-int-distribution{a:1,b:3}"));

    EXPECT(*a != *a);
    EXPECT(*a != *make(add_random("uniform-int-distribution{a:1,b:3}")));
}


}  // namespace mir::action::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
