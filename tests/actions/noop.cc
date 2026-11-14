// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "action.h"


namespace mir::tests::actions {


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


}  // namespace mir::tests::actions


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
