// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <string>

#include "eckit/testing/Test.h"

#include "mir/data/MIRField.h"
#include "mir/input/GribFileInput.h"
#include "mir/input/VectorInput.h"

#include "input.h"


namespace mir::api::tests {


CASE("VectorInput") {
    input::GribFileInput u("tiny-2x2.grib", 0, 2);
    input::GribFileInput v("tiny-2x2.grib", 1, 2);
    input::VectorInput vector(u, v);

    MIRInput& in = vector;
    EXPECT(in.next());
    EXPECT_EQUAL(in.dimensions(), 2);

    auto field = in.field();
    EXPECT_EQUAL(field.dimensions(), 2);
    EXPECT_EQUAL(field.values(0).size(), 4);
    EXPECT_EQUAL(field.values(1).size(), 4);

    // each component has its own parametrisation and handle
    long edition0 = 0;
    long edition1 = 0;
    EXPECT(in.parametrisation(0).get("edition", edition0) && edition0 == 1);
    EXPECT(in.parametrisation(1).get("edition", edition1) && edition1 == 2);
    EXPECT(in.gribHandle(0) != in.gribHandle(1));

    input::GribFileInput u2("tiny-2x2.grib", 0, 2);
    input::GribFileInput v2("tiny-2x2.grib", 1, 2);
    EXPECT(in.sameAs(input::VectorInput(u2, v2)));
    EXPECT_NOT(in.sameAs(u));

    EXPECT(to_string(in).find("VectorInput[GribFileInput[") == 0);
    EXPECT_NOT(in.next());
}


}  // namespace mir::api::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
