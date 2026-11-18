// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <string>
#include <vector>

#include "eckit/testing/Test.h"

#include "mir/data/MIRField.h"
#include "mir/input/GribFileInput.h"
#include "mir/input/MultiStreamInput.h"

#include "input.h"


namespace mir::api::tests {


CASE("MultiStreamInput") {
    auto* multi = new input::MultiStreamInput;
    input_t in(multi);

    // components are not owned
    input::GribFileInput a("tiny-2x2.grib");
    input::GribFileInput b("tiny-3x3.grib");
    multi->append(&a);
    multi->append(&b);

    // the first stream's dimensions and parametrisation
    MIRInput& first = a;
    EXPECT(first.next());
    EXPECT_EQUAL(in->dimensions(), 1);
    EXPECT(&in->parametrisation() == &first.parametrisation());
    EXPECT(in->gribHandle() == first.gribHandle());
    EXPECT_EQUAL(in->field().values(0).size(), 4);

    auto* same = new input::MultiStreamInput;
    input_t other(same);
    same->append(&a);
    same->append(&b);
    EXPECT(in->sameAs(*other));

    auto* shorter = new input::MultiStreamInput;
    input_t another(shorter);
    shorter->append(&a);
    EXPECT_NOT(in->sameAs(*another));

    EXPECT(to_string(*in).find("MultiStreamInput[GribFileInput[") == 0);
}


}  // namespace mir::api::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
