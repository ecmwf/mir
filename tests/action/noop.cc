// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "action.h"


namespace mir::action::tests {


CASE("noop") {
    auto in  = field("{grid: [10, 10]}", [](double lat, double lon) { return lat + lon; });
    auto out = perform("noop", in);

    EXPECT(out.representation()->sameAs(*in.representation()));
    EXPECT(out.values(0) == in.values(0));
}


CASE("noop, comparison") {
    EXPECT(*make("noop") == *make("{type: noop}"));
    EXPECT(*make("noop") != *make("misc.reset-missing-values"));
}


CASE("noop, printing") {
    EXPECT_EQUAL(str(*make("noop")), "Noop[]");
    EXPECT_EQUAL(custom_str(*make("noop")), "Noop[]");
}


}  // namespace mir::action::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
