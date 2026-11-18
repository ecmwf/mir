// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <string>

#include "eckit/testing/Test.h"

#include "mir/data/MIRField.h"
#include "mir/input/EmptyInput.h"

#include "input.h"


namespace mir::api::tests {


CASE("EmptyInput") {
    input_t in(new input::EmptyInput);
    EXPECT(in->next());
    EXPECT_NOT(in->next());

    std::string gridType;
    EXPECT(in->parametrisation().get("gridType", gridType) && gridType == "none");

    EXPECT(in->sameAs(input::EmptyInput()));
    EXPECT_NOT(in->sameAs(*build("/dev/null", "{artificialInput: dummy}")));
    EXPECT_EQUAL(to_string(*in), "EmptyInput[...]");
    EXPECT_NO_THROW(in->field());
}


}  // namespace mir::api::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
