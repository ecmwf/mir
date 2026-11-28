// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <sstream>
#include <string>

#include "eckit/exception/Exceptions.h"
#include "eckit/log/JSON.h"
#include "eckit/testing/Test.h"

#include "mir/action/context/Context.h"
#include "mir/data/MIRField.h"
#include "mir/input/VectorInput.h"
#include "mir/output/ValuesOutput.h"
#include "mir/output/VectorOutput.h"
#include "mir/param/CombinedParametrisation.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/MIRStatistics.h"

#include "output.h"


namespace mir::api::tests {


CASE("VectorOutput") {
    GridInput u("{grid: [10, 10], area: [10, 0, 0, 10]}");
    GridInput v("{grid: [10, 10], area: [10, 0, 0, 10]}");
    for (auto& x : v.values) {
        x += 100.;
    }

    input::VectorInput vector(*u.input, *v.input);
    util::MIRStatistics statistics;

    output::ValuesOutput out1;
    output::ValuesOutput out2;
    output::VectorOutput out(out1, out2);
    MIROutput& base = out;

    param::SimpleParametrisation user;
    const param::CombinedParametrisation param(user);


    SECTION("save") {
        context::Context ctx(vector, statistics);
        EXPECT_EQUAL(ctx.field().dimensions(), 2);

        base.save(param, ctx);
        EXPECT(out1.values() == u.values);
        EXPECT(out2.values() == v.values);
    }


    SECTION("copy, set") {
        context::Context ctx1(vector, statistics);
        base.copy(param, ctx1);
        EXPECT(out1.values() == u.values);
        EXPECT(out2.values() == v.values);

        output::ValuesOutput out3;
        output::ValuesOutput out4;
        output::VectorOutput another(out3, out4);

        context::Context ctx2(vector, statistics);
        static_cast<MIROutput&>(another).set(param, ctx2);
        EXPECT(out4.values() == v.values);
    }


    SECTION("not a vector input") {
        context::Context ctx(*u.input, statistics);
        EXPECT_THROWS_AS(base.copy(param, ctx), exception::SeriousBug);
        EXPECT_THROWS_AS(base.set(param, ctx), exception::SeriousBug);

        GridInput two("{grid: [10, 10], area: [10, 0, 0, 10]}", 2);
        context::Context ctx2(*two.input, statistics);
        EXPECT_THROWS_AS(base.save(param, ctx2), exception::SeriousBug);
    }


    SECTION("comparisons") {
        output::VectorOutput same(out1, out2);
        output::VectorOutput swapped(out2, out1);
        EXPECT(base.sameAs(same));
        EXPECT_NOT(base.sameAs(swapped));
        EXPECT_NOT(base.sameAs(out1));

        EXPECT(base.sameParametrisation(param, param));

        std::ostringstream str;
        EXPECT_NOT(base.printParametrisation(str, param));

        std::ostringstream json;
        eckit::JSON j(json);
        j.startObject();
        base.jsonParametrisation(j, param);
        j.endObject();
        EXPECT_EQUAL(json.str(), "{}");

        EXPECT_EQUAL(to_string(base), "VectorOutput[ValuesOutput[],ValuesOutput[]]");
    }
}


}  // namespace mir::api::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
