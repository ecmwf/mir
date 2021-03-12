/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include <algorithm>
#include <ios>
#include <memory>
#include <string>
#include <vector>

#include "eckit/testing/Test.h"

#include "mir/action/calc/FormulaAction.h"
#include "mir/action/plan/Action.h"
#include "mir/action/plan/ActionPlan.h"
#include "mir/api/mir_config.h"
#include "mir/data/MIRField.h"
#include "mir/input/MIRInput.h"
#include "mir/key/style/MIRStyle.h"
#include "mir/output/EmptyOutput.h"
#include "mir/param/CombinedParametrisation.h"
#include "mir/param/DefaultParametrisation.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/util/Log.h"

// define EXPECTV(a) log << "\tEXPECT(" << #a <<")" << std::endl; EXPECT(a)


namespace mir {
namespace tests {
namespace unit {


bool plan_has_action(const action::ActionPlan& plan, const action::Action& action) {
    return (plan.end() != std::find_if(plan.begin(), plan.end(), [&](const action::Action* test) {
                ASSERT(test);
                return test->sameAs(action);
            }));
}


class InputOutput : public param::SimpleParametrisation {
    const std::vector<double> grid{1, 1};

protected:
    InputOutput(bool gridded) { gridded ? set("grid", grid).set("gridded", true) : set("spectral", 1); }
};


struct TestingInput : input::MIRInput, InputOutput {
    TestingInput(bool gridded) : InputOutput(gridded) {}

private:
    const param::MIRParametrisation& parametrisation(size_t) const override { return *this; }
    bool sameAs(const MIRInput&) const override { return false; }
    data::MIRField field() const override { NOTIMP; }
    void print(std::ostream& out) const override { InputOutput::print(out); }
    friend std::ostream& operator<<(std::ostream& s, const TestingInput& p) {
        p.print(s);
        return s;
    }
};


struct TestingOutput : InputOutput {
    TestingOutput(bool gridded) : InputOutput(gridded) {}
};


CASE("ECMWFStyle") {

    Log::info() << std::boolalpha;
    static const param::DefaultParametrisation defaults;

    std::vector<bool> _no_yes{false, true};
    std::vector<bool> _yes_no{true, false};
    std::vector<std::string> _when{"prologue", "gridded", "spectral", "raw", "epilogue"};


    SECTION("mir::action::FormulaAction") {

        const std::string CORRECT_FORMULA("1");
        const std::string WRONG_FORMULA("0");
        const std::string CORRECT_METADATA("param=1");
        const std::string WRONG_METADATA("param=0");

        param::SimpleParametrisation p1;
        param::SimpleParametrisation p2;
        param::SimpleParametrisation p3;
        param::SimpleParametrisation p4;

        p1.set("formula", CORRECT_FORMULA).set("formula.metadata", WRONG_METADATA);
        p2.set("formula", WRONG_FORMULA).set("formula.metadata", CORRECT_METADATA);
        p3.set("formula", WRONG_FORMULA).set("formula.metadata", WRONG_METADATA);
        p4.set("formula", CORRECT_FORMULA).set("formula.metadata", CORRECT_METADATA);

        const action::FormulaAction WRONG_ACTION_1(p1);
        const action::FormulaAction WRONG_ACTION_2(p2);
        const action::FormulaAction WRONG_ACTION_3(p3);
        const action::FormulaAction CORRECT_ACTION(p4);


#if defined(mir_HAVE_ATLAS)
        for (bool input_gridded : _yes_no) {
#else
        for (bool input_gridded : {true}) {
#endif
            TestingInput in(input_gridded);

            for (bool output_gridded : _yes_no) {
                if (input_gridded && !output_gridded) {
                    // this combination isn't supported
                    continue;
                }

                for (const std::string& when : _when) {

                    TestingOutput user(output_gridded);
                    user.set("formula." + when, CORRECT_FORMULA);
                    user.set("formula." + when + ".metadata", CORRECT_METADATA);

                    // test correct user formula.<when> and formula.<when>.metadata, then
                    // test extra, inconsistent formula.<when>.metadata options
                    bool plan_should_have_formula = when == "gridded"    ? (input_gridded || output_gridded)
                                                    : when == "spectral" ? (!input_gridded || !output_gridded)
                                                                         : true;

                    for (bool addWrongArguments : _no_yes) {
                        if (addWrongArguments) {
                            for (const std::string& wrongWhen : _when) {
                                if (wrongWhen != when) {
                                    user.set("formula." + wrongWhen + ".metadata", WRONG_METADATA);
                                }
                            }
                            if (input_gridded && output_gridded) {
                                user.set("formula.spectral", WRONG_FORMULA);
                            }
                            if (!input_gridded && !output_gridded) {
                                user.set("formula.gridded", WRONG_FORMULA);
                            }
                        }

                        output::EmptyOutput out;

                        const param::CombinedParametrisation combined(user, in, defaults);
                        std::unique_ptr<key::style::MIRStyle> style(key::style::MIRStyleFactory::build(combined));

                        action::ActionPlan plan(combined);
                        style->prepare(plan, in, out);

                        bool plan_has_this_formula   = plan_has_action(plan, CORRECT_ACTION);
                        bool plan_has_wrong_formulae = plan_has_action(plan, WRONG_ACTION_1) ||
                                                       plan_has_action(plan, WRONG_ACTION_2) ||
                                                       plan_has_action(plan, WRONG_ACTION_3);

                        static size_t c = 1;
                        Log::info() << "Test " << c++ << ":"
                                    << "\n\t"
                                       "formula."
                                    << when << ", formula." << when << ".metadata"
                                    << "\n\t"
                                       "in:   "
                                    << in
                                    << "\n\t"
                                       "user: "
                                    << user
                                    << "\n\t"
                                       "plan: "
                                    << plan
                                    << "\n\t"
                                       "has "
                                    << when << " formula: " << plan_has_this_formula << " (should be "
                                    << plan_should_have_formula << ")" << std::endl;

                        EXPECT(plan_has_this_formula == plan_should_have_formula);
                        EXPECT(!plan_has_wrong_formulae);
                    }
                }
            }
        }
    }
}


}  // namespace unit
}  // namespace tests
}  // namespace mir


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
