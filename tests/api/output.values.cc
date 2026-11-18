// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <sstream>
#include <string>

#include "eckit/exception/Exceptions.h"
#include "eckit/testing/Test.h"

#include "mir/api/MIRJob.h"
#include "mir/data/MIRField.h"
#include "mir/output/ValuesOutput.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/util/Exceptions.h"

#include "output.h"


namespace mir::api::tests {


CASE("ValuesOutput") {
    GridInput in("{grid: [10, 10], area: [10, 0, -10, 20]}");

    output::ValuesOutput out;
    MIROutput& base = out;

    MIRJob job;
    job.set("caching", false);
    job.execute(*in, out);

    EXPECT_EQUAL(out.dimensions(), 1);
    EXPECT(out.values() == in.values);
    EXPECT_NOT(out.hasMissing());
    EXPECT_EQUAL(out.missingValue(), 9999.);
    EXPECT_THROWS_AS(out.values(1), eckit::AssertionFailed);

    EXPECT(base.sameAs(out));
    EXPECT_NOT(base.sameAs(output::ValuesOutput()));
    EXPECT_EQUAL(to_string(base), "ValuesOutput[]");

    param::SimpleParametrisation p;
    EXPECT(base.sameParametrisation(p, p));

    std::ostringstream str;
    EXPECT_NOT(base.printParametrisation(str, p));


    SECTION("missing values") {
        GridInput mv("{grid: [10, 10], area: [10, 0, -10, 20]}");
        mv.metadata.set("missing_value", 2.);

        output::ValuesOutput out2;
        job.execute(*mv, out2);
        EXPECT(out2.hasMissing());
        EXPECT_EQUAL(out2.missingValue(), 2.);
    }
}


}  // namespace mir::api::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
