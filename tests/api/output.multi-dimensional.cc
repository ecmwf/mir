// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <memory>
#include <sstream>
#include <string>

#include "eckit/exception/Exceptions.h"
#include "eckit/log/JSON.h"
#include "eckit/testing/Test.h"

#include "mir/action/context/Context.h"
#include "mir/data/MIRField.h"
#include "mir/input/MultiDimensionalInput.h"
#include "mir/input/RawInput.h"
#include "mir/output/EmptyOutput.h"
#include "mir/output/MultiDimensionalOutput.h"
#include "mir/output/ValuesOutput.h"
#include "mir/param/CombinedParametrisation.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/MIRStatistics.h"

#include "output.h"


namespace mir::api::tests {


CASE("MultiDimensionalOutput") {
    GridInput a("{grid: [10, 10], area: [10, 0, 0, 10]}");
    GridInput b("{grid: [10, 10], area: [10, 0, 0, 10]}");
    for (auto& x : b.values) {
        x += 100.;
    }

    // components are owned
    auto* multi = new input::MultiDimensionalInput;
    std::unique_ptr<input::MIRInput> input(multi);
    multi->append(new input::RawInput(a.values.data(), a.values.size(), a.metadata));
    multi->append(new input::RawInput(b.values.data(), b.values.size(), b.metadata));
    ASSERT(input->next());

    auto* out1 = new output::ValuesOutput;
    auto* out2 = new output::ValuesOutput;
    output::MultiDimensionalOutput out;
    out.appendDimensionalOutput(out1);
    out.appendDimensionalOutput(out2);
    MIROutput& base = out;

    util::MIRStatistics statistics;
    param::SimpleParametrisation user;
    const param::CombinedParametrisation param(user);


    SECTION("save, copy, set") {
        context::Context ctx(*input, statistics);
        EXPECT_EQUAL(ctx.field().dimensions(), 2);
        base.save(param, ctx);
        EXPECT(out1->values() == a.values);
        EXPECT(out2->values() == b.values);

        context::Context ctx1(*input, statistics);
        base.copy(param, ctx1);
        EXPECT(out2->values() == b.values);

        context::Context ctx2(*input, statistics);
        base.set(param, ctx2);
        EXPECT(out1->values() == a.values);
    }


    SECTION("not a multi-dimensional input") {
        context::Context ctx(*a.input, statistics);
        EXPECT_THROWS_AS(base.copy(param, ctx), exception::SeriousBug);
        EXPECT_THROWS_AS(base.save(param, ctx), exception::SeriousBug);
        EXPECT_THROWS_AS(base.set(param, ctx), exception::SeriousBug);
    }


    SECTION("comparisons") {
        EXPECT(base.sameParametrisation(param, param));

        std::ostringstream str;
        EXPECT_NOT(base.printParametrisation(str, param));

        std::ostringstream json;
        eckit::JSON j(json);
        j.startObject();
        base.jsonParametrisation(j, param);
        j.endObject();
        EXPECT_EQUAL(json.str(), "{}");

        output::MultiDimensionalOutput shorter;
        shorter.appendDimensionalOutput(new output::ValuesOutput);
        EXPECT_NOT(base.sameAs(shorter));
        EXPECT_NOT(base.sameAs(*out1));

        EXPECT(to_string(base).find("MultiDimensionalOutput[") == 0);
    }
}


}  // namespace mir::api::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
