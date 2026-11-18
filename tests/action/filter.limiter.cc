// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <algorithm>
#include <cstdlib>

#include "action.h"


namespace mir::action::tests {


CASE("filter.limiter") {
    auto in = field("{grid: [10, 10]}", [](double lat, double) { return lat; });

    SECTION("minimum and maximum") {
        auto out      = perform("{type: filter.limiter, limiter-minimum: -10, limiter-maximum: 20}", in);
        const auto& v = out.values(0);

        EXPECT(*std::min_element(v.begin(), v.end()) == -10.);
        EXPECT(*std::max_element(v.begin(), v.end()) == 20.);
    }

    SECTION("missing values are kept") {
        auto miss = field("{grid: [10, 10]}", [](double lat, double) { return lat > 50 ? 9999. : lat; }, 1, 9999.);
        auto out  = perform("{type: filter.limiter, limiter-minimum: 0, limiter-maximum: 1}", miss);

        EXPECT_EQUAL(count_missing(out), count_missing(miss));
    }

    SECTION("as configured, by parameter") {
        auto out      = perform("{type: filter.limiter, paramId: 29}", in);
        const auto& v = out.values(0);

        EXPECT(*std::min_element(v.begin(), v.end()) == 1.);
        EXPECT(*std::max_element(v.begin(), v.end()) == 20.);
    }

    SECTION("as configured, by parameter, as given") {
        auto out      = perform("{type: filter.limiter, paramId: 29, limiter-minimum: 0, limiter-maximum: 2}", in);
        const auto& v = out.values(0);

        EXPECT(*std::min_element(v.begin(), v.end()) == 0.);
        EXPECT(*std::max_element(v.begin(), v.end()) == 2.);
    }

    SECTION("as configured, unknown parameter: unlimited") {
        auto out = perform("{type: filter.limiter, paramId: 167}", in);

        EXPECT(out.values(0) == in.values(0));
    }
}


CASE("filter.limiter, errors") {
    EXPECT_THROWS(make("{type: filter.limiter, limiter-minimum: 1, limiter-maximum: 0}"));
}


CASE("filter.limiter, printing") {
    EXPECT_EQUAL(str(*make("{type: filter.limiter, limiter-minimum: -10, limiter-maximum: 20}")),
                 "Limiter[minimum=-10,maximum=20]");
}


}  // namespace mir::action::tests


int main(int argc, char** argv) {
    // limits by parameter, as configured for testing (filter.limiter)
    ::setenv("MIR_CONFIG_LIMITER", "~mir/etc/mir/limiter-test.yaml", 1);
    return eckit::testing::run_tests(argc, argv);
}
