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
#include "eckit/utils/StringTools.h"

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
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"

// define EXPECTV(a) log << "\tEXPECT(" << #a <<")" << std::endl; EXPECT(a)


namespace mir::tests::unit {


bool plan_has_action(const action::ActionPlan& plan, const action::Action& action) {
    return (plan.end() != std::find_if(plan.begin(), plan.end(), [&](const action::Action* test) {
                ASSERT(test);
                return test->sameAs(action);
            }));
}


class InputOutput : public param::SimpleParametrisation {
    const std::vector<double> grid{1, 1};

protected:
    explicit InputOutput(bool gridded) {
        gridded ? set("grid", grid).set("gridded", true) : set("truncation", 1).set("spectral", true);
    }
};


struct TestingInput : input::MIRInput, InputOutput {
    explicit TestingInput(bool gridded) : InputOutput(gridded) {}

private:
    const param::MIRParametrisation& parametrisation(size_t /*which*/) const override { return *this; }
    bool sameAs(const MIRInput& /*unused*/) const override { return false; }
    data::MIRField field() const override { NOTIMP; }
    void print(std::ostream& out) const override { InputOutput::print(out); }
    friend std::ostream& operator<<(std::ostream& s, const TestingInput& p) {
        p.print(s);
        return s;
    }
};


struct TestingOutput : InputOutput {
    explicit TestingOutput(bool gridded) : InputOutput(gridded) {}
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


#if mir_HAVE_ATLAS
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

                        const param::CombinedParametrisation param(user, in, defaults);
                        std::unique_ptr<key::style::MIRStyle> style(key::style::MIRStyleFactory::build(param));

                        action::ActionPlan plan(param);
                        style->prepare(plan, out);

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


    SECTION("mir::action::Area") {
        auto plan_contains = [](const action::ActionPlan& plan, const std::string& action) {
            return std::any_of(plan.begin(), plan.end(), [&action](const action::Action* act_ptr) -> bool {
                std::ostringstream str;
                str << *act_ptr;
                return eckit::StringTools::startsWith(str.str(), action);
            });
        };

        // trigger post-processing, but avoid the same points
        param::SimpleParametrisation user;
        user.set("grid", std::vector<double>{2, 2});
        user.set("area", std::vector<double>{90, 0.1, -90, 360});

        TestingInput in(true);
        output::EmptyOutput out;

        const param::CombinedParametrisation param(user, in, defaults);
        std::unique_ptr<key::style::MIRStyle> style(key::style::MIRStyleFactory::build(param));


        Log::info() << "area-mode=crop for regular lat/lon" << std::endl;
        {
            user.set("area-mode", "crop");
            user.clear("rotation");

            action::ActionPlan plan(param);
            style->prepare(plan, out);
            EXPECT(plan_contains(plan, "AreaCropper"));
        }


        Log::info() << "area-mode=mask for regular lat/lon" << std::endl;
        {
            user.set("area-mode", "mask");
            user.clear("rotation");

            action::ActionPlan plan(param);
            style->prepare(plan, out);
            EXPECT(plan_contains(plan, "AreaMasker"));
        }


        Log::info() << "area-mode=crop for rotated regular lat/lon" << std::endl;
        {
            user.set("area-mode", "crop");
            user.set("rotation", std::vector<double>{-90, 0});

            action::ActionPlan plan(param);
            style->prepare(plan, out);
            EXPECT(plan_contains(plan, "AreaCropper"));
        }


        Log::info() << "area-mode=mask for rotated regular lat/lon" << std::endl;
        {
            user.set("area-mode", "mask");
            user.set("rotation", std::vector<double>{-90, 0});

            action::ActionPlan plan(param);
            EXPECT_THROWS_AS(style->prepare(plan, out), exception::UserError);
        }
    }
}


}  // namespace mir::tests::unit


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
