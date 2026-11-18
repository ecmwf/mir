// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <string>

#include "action.h"


namespace mir::action::tests {


CASE("select.field") {
    auto in = field("{grid: [10, 10]}");

    for (size_t which = 0; which < 3; ++which) {
        MIRValuesVector values(in.values(0).size(), static_cast<double>(which));
        in.update(values, which);
    }

    EXPECT_EQUAL(in.dimensions(), 3);

    for (size_t which = 0; which < 3; ++which) {
        SECTION("which: " + std::to_string(which)) {
            auto out = perform("{type: select.field, which: " + std::to_string(which) + "}", in);

            EXPECT_EQUAL(out.dimensions(), 1);
            EXPECT(out.values(0).front() == static_cast<double>(which));
        }
    }
}


CASE("select.field, comparison and errors") {
    EXPECT(*make("{type: select.field, which: 1}") != *make("{type: select.field, which: 0}"));
    EXPECT_THROWS(make("select.field"));  // which is required
}


CASE("select.field, printing") {
    EXPECT_EQUAL(str(*make("{type: select.field, which: 1}")), "SelectField[1]");
}


}  // namespace mir::action::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
