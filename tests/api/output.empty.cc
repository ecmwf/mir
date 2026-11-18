// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <sstream>
#include <string>
#include <vector>

#include "eckit/log/JSON.h"
#include "eckit/testing/Test.h"

#include "mir/api/MIRJob.h"
#include "mir/output/EmptyOutput.h"
#include "mir/output/ValuesOutput.h"
#include "mir/param/CombinedParametrisation.h"
#include "mir/param/SimpleParametrisation.h"

#include "output.h"


namespace mir::api::tests {


CASE("EmptyOutput") {
    GridInput in("{grid: [10, 10]}");

    output::EmptyOutput out;
    MIROutput& base = out;

    MIRJob job;
    job.set("caching", false);
    EXPECT_NO_THROW(job.execute(*in, out));

    EXPECT(base.sameAs(output::EmptyOutput("path")));
    EXPECT_NOT(base.sameAs(output::ValuesOutput()));
    EXPECT_EQUAL(to_string(base), "EmptyOutput[]");

    param::SimpleParametrisation field;
    param::SimpleParametrisation user;
    const param::CombinedParametrisation param(user, field);
    EXPECT_NOT(base.sameParametrisation(param, param));

    std::ostringstream none;
    EXPECT_NOT(base.printParametrisation(none, param));
    EXPECT(none.str().empty());

    user.set("accuracy", 12L).set("packing", "ccsds").set("edition", 2L);

    std::ostringstream all;
    EXPECT(base.printParametrisation(all, param));
    EXPECT_EQUAL(all.str(), "accuracy=12,packing=ccsds,edition=2");

    std::ostringstream json;
    eckit::JSON j(json);
    j.startObject();
    base.jsonParametrisation(j, param);
    j.endObject();
    EXPECT_EQUAL(json.str(), R"({"accuracy":12,"packing":"ccsds","edition":2})");

    for (const auto& key : std::vector<std::string>{"packing", "edition"}) {
        param::SimpleParametrisation one;
        one.set(key, key == "packing" ? std::string("simple") : std::string("1"));
        const param::CombinedParametrisation p(one, field);

        std::ostringstream str;
        EXPECT(base.printParametrisation(str, p));
        EXPECT(str.str().find(key + "=") == 0);
    }
}


}  // namespace mir::api::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
