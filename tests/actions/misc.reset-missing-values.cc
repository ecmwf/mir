// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <limits>

#include "action.h"


namespace mir::tests::actions {


CASE("misc.reset-missing-values") {
    const auto lowest = std::numeric_limits<double>::lowest();

    SECTION("first value repeated: missing values") {
        auto in  = field("{grid: [10, 10]}", [](double lat, double) { return lat > 0 ? 1234. : lat; });
        auto out = perform("misc.reset-missing-values", in);

        EXPECT(out.hasMissing());
        EXPECT(out.missingValue() == lowest);
        EXPECT(count_missing(out) > 0);
        EXPECT(count_missing(out) < out.values(0).size());
    }

    SECTION("constant field: no missing values") {
        auto out = perform("misc.reset-missing-values", field("{grid: [10, 10]}", [](double, double) { return 1.; }));

        EXPECT(!out.hasMissing());
    }

    SECTION("field with missing values: unchanged") {
        auto in = field("{grid: [10, 10]}", [](double lat, double) { return lat > 0 ? 9999. : lat; }, 1, 9999.);

        EXPECT(in.hasMissing());

        auto out = perform("misc.reset-missing-values", in);

        EXPECT(out.missingValue() == 9999.);
        EXPECT(out.values(0) == in.values(0));
    }
}


}  // namespace mir::tests::actions


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
