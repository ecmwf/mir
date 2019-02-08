/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include <algorithm>
#include <ios>
#include <string>
#include <vector>

#include "eckit/log/Log.h"
#include "eckit/memory/ScopedPtr.h"
#include "eckit/testing/Test.h"

#include "mir/action/calc/FormulaAction.h"
#include "mir/action/plan/Action.h"
#include "mir/action/plan/ActionPlan.h"
#include "mir/api/MIRWatcher.h"
#include "mir/param/CombinedParametrisation.h"
#include "mir/param/DefaultParametrisation.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/style/MIRStyle.h"

//define EXPECTV(a) log << "\tEXPECT(" << #a <<")" << std::endl; EXPECT(a)


namespace mir {
namespace tests {
namespace unit {


bool plan_has_action(const action::ActionPlan& plan, const action::Action& action) {
    for (size_t i = 0; i < plan.size(); ++ i) {
        if (plan.action(i).sameAs(action)) {
                return true;
        }
    }
    return false;
}


CASE("ECMWFStyle") {

    eckit::Log::info() << std::boolalpha;
    static const param::DefaultParametrisation defaults;


    SECTION("mir::action::FormulaAction") {

        static const std::string
                CORRECT_FORMULA  ("1"),
                WRONG_FORMULA    ("0"),
                CORRECT_METADATA ("param=1"),
                WRONG_METADATA   ("param=0");

        param::SimpleParametrisation p1, p2, p3, p4;
        const action::FormulaAction
                WRONG_ACTION_1(p1.set("formula", CORRECT_FORMULA).set("formula.metadata", WRONG_METADATA)),
                WRONG_ACTION_2(p2.set("formula", WRONG_FORMULA).set("formula.metadata", CORRECT_METADATA)),
                WRONG_ACTION_3(p3.set("formula", WRONG_FORMULA).set("formula.metadata", WRONG_METADATA)),
                CORRECT_ACTION(p4.set("formula", CORRECT_FORMULA).set("formula.metadata", CORRECT_METADATA));


        class InputOutput : public param::SimpleParametrisation {
            const std::vector<double> grid{ 1, 1 };
        public:
            InputOutput(bool gridded) {
                gridded ? set("grid", grid).set("gridded", true) : set("spectral", 1);
            }
        };


        for (bool input_gridded : {true, false}) {
            const InputOutput in(input_gridded);

            for (bool output_gridded : {true, false}) {
                if (input_gridded && !output_gridded) {
                    // this combination isn't supported
                    continue;
                }

                for (const std::string& when : {"prologue", "gridded", "spectral", "raw", "epilogue"}) {

                    InputOutput out(output_gridded);
                    out.set("formula." + when, CORRECT_FORMULA);
                    out.set("formula." + when + ".metadata", CORRECT_METADATA);

                    // test correct user formula.<when> and formula.<when>.metadata, then
                    // test extra, inconsistent formula.<when>.metadata options
                    bool plan_should_have_formula = when == "gridded" ? (input_gridded || output_gridded)
                                                  : when == "spectral" ? (!input_gridded || !output_gridded)
                                                  : true;

                    for (bool addWrongArguments : {false, true}) {
                        if (addWrongArguments) {
                            for (const std::string& wrongWhen : {"prologue", "gridded", "spectral", "raw", "epilogue"}) {
                                if (wrongWhen != when) {
                                    out.set("formula." + wrongWhen + ".metadata", WRONG_METADATA);
                                }
                            }
                            if (input_gridded && output_gridded) {
                                out.set("formula.spectral", WRONG_FORMULA);
                            }
                            if (!input_gridded && !output_gridded) {
                                out.set("formula.gridded", WRONG_FORMULA);
                            }
                        }

                        const param::CombinedParametrisation combined(out, in, defaults);
                        eckit::ScopedPtr<style::MIRStyle> style(style::MIRStyleFactory::build(combined));

                        action::ActionPlan plan(combined);
                        style->prepare(plan);

                        bool plan_has_this_formula = plan_has_action(plan, CORRECT_ACTION);
                        bool plan_has_wrong_formulae =
                                plan_has_action(plan, WRONG_ACTION_1) ||
                                plan_has_action(plan, WRONG_ACTION_2) ||
                                plan_has_action(plan, WRONG_ACTION_3);

                        static size_t c = 1;
                        eckit::Log::info() << "Test " << c++ << ":"
                                           << "\n\t" "formula." << when << ", formula." << when << ".metadata"
                                           << "\n\t" "in:   " << in
                                           << "\n\t" "user: " << out
                                           << "\n\t" "plan: " << plan
                                           << "\n\t" "has " << when << " formula: " << plan_has_this_formula << " (should be " << plan_should_have_formula << ")"
                                           << std::endl;

                        EXPECT(plan_has_this_formula == plan_should_have_formula);
                        EXPECT(!plan_has_wrong_formulae);
                    }

                }
            }
        }

    }
}


} // namespace unit
} // namespace tests
} // namespace mir


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
