// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <fstream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "eckit/filesystem/TmpFile.h"
#include "eckit/testing/Test.h"

#include "mir/action/plan/ActionPlan.h"
#include "mir/api/MIRJob.h"
#include "mir/key/style/CustomParametrisation.h"
#include "mir/key/style/MIRStyle.h"
#include "mir/output/EmptyOutput.h"
#include "mir/param/DefaultParametrisation.h"
#include "mir/param/RuntimeParametrisation.h"
#include "mir/param/SimpleParametrisation.h"

#include "style.h"


namespace mir::action::tests {


CASE("custom style") {
    const std::string PLAN =
        "frame(2);crop.area(area=[60,-10,30,40]);filter.statistics(statistics=[min-max,scalar],precision=3)";

    auto check = [](const action::ActionPlan& plan) {
        EXPECT_EQUAL(plan.size(), 4);
        EXPECT_EQUAL(plan.action(0).json_str(), R"({"type":"filter.frame","frame":2})");
        EXPECT_EQUAL(plan.action(1).json_str(), R"({"type":"crop.area","area":[60,-10,30,40]})");
        EXPECT_EQUAL(plan.action(2).json_str(),
                     R"({"type":"filter.statistics","statistics":"min-max/scalar","precision":3})");
        EXPECT_EQUAL(plan.action(3).type(), "save");
    };

    api::MIRJob job;
    job.set("style", "custom");

    SECTION("plan, actions by (partial) name") {
        // the second time, partial names are known
        check(prepared_t(O32, job.set("plan", PLAN)).job.plan());
        check(prepared_t(O32, job.set("plan", PLAN)).job.plan());
    }

    SECTION("plan script") {
        eckit::TmpFile script;
        {
            std::ofstream out(script.asString());
            out << PLAN;
        }

        check(prepared_t(O32, job.set("plan-script", script.asString())).job.plan());
    }

    SECTION("errors") {
        EXPECT_THROWS(types(O32, job));  // no plan
        EXPECT_THROWS(types(O32, job.set("plan-script", "does-not-exist.plan")));
        EXPECT_THROWS(types(O32, job.set("plan", "area(area=[60,-10,30,40])")));  // ambiguous action
        EXPECT_THROWS(types(O32, job.set("plan", "unknown(1)")));
    }
}


CASE("custom style, parametrisation") {
    param::SimpleParametrisation base;
    base.set("base", 1L);

    const std::map<std::string, std::vector<std::string>> params{
        {"text", {"a", "b"}}, {"number", {"2"}}, {"real", {"2.5"}}, {"flag", {"1"}}, {"list", {"1", "2", "3"}}};

    const key::style::CustomParametrisation custom("test", params, base);
    const param::MIRParametrisation& parametrisation = custom;

    EXPECT(&parametrisation.userParametrisation() == &parametrisation);
    EXPECT(&parametrisation.fieldParametrisation() == &parametrisation);

    std::ostringstream print;
    print << parametrisation;

    EXPECT(starts(print.str(), "CustomParametrisation[name=test,params=["));
    EXPECT(contains(print.str(), "list=[1,2,3]"));

    // values as given, by type
    std::string text;
    EXPECT(parametrisation.get("text", text) && text == "a/b");

    int i  = 0;
    long l = 0;
    EXPECT(parametrisation.get("number", i) && i == 2);
    EXPECT(parametrisation.get("number", l) && l == 2);

    float f  = 0;
    double d = 0;
    EXPECT(parametrisation.get("real", f) && f == 2.5F);
    EXPECT(parametrisation.get("real", d) && d == 2.5);

    bool b = false;
    EXPECT(parametrisation.get("flag", b) && b);

    std::vector<int> vi;
    std::vector<long> vl;
    std::vector<float> vf;
    std::vector<double> vd;
    EXPECT(parametrisation.get("list", vi) && vi == std::vector<int>({1, 2, 3}));
    EXPECT(parametrisation.get("list", vl) && vl == std::vector<long>({1, 2, 3}));
    EXPECT(parametrisation.get("list", vf) && vf == std::vector<float>({1, 2, 3}));
    EXPECT(parametrisation.get("list", vd) && vd == std::vector<double>({1, 2, 3}));

    std::vector<std::string> vs;
    EXPECT(parametrisation.get("text", vs) && vs == std::vector<std::string>({"a", "b"}));

    // one value expected
    EXPECT_THROWS(parametrisation.get("text", l));

    // otherwise, from the base parametrisation
    EXPECT(parametrisation.has("number"));
    EXPECT(parametrisation.has("base"));
    EXPECT(!parametrisation.has("none"));
    EXPECT(parametrisation.get("base", l) && l == 1);
    EXPECT(!parametrisation.get("none", text));
}


CASE("styles, factory") {
    param::SimpleParametrisation param;
    EXPECT_THROWS(std::unique_ptr<key::style::MIRStyle>(key::style::MIRStyleFactory::build(param)));  // no style

    param.set("style", "unknown");
    EXPECT_THROWS(std::unique_ptr<key::style::MIRStyle>(key::style::MIRStyleFactory::build(param)));

    std::ostringstream list;
    key::style::MIRStyleFactory::list(list);

    EXPECT(contains(list.str(), "custom"));
    EXPECT(contains(list.str(), "ecmwf"));

    struct test_t {
        std::string style;
        std::string print;
    };

    const std::vector<test_t> tests{{"ecmwf", "ECMWFStyle[]"}, {"custom", "CustomStyle[]"}};
    for (const auto& test : tests) {
        param.set("style", test.style);
        std::unique_ptr<key::style::MIRStyle> style(key::style::MIRStyleFactory::build(param));

        std::ostringstream print;
        print << *style;
        EXPECT_EQUAL(print.str(), test.print);
    }

    // custom style, no plan
    param.set("style", "custom");
    std::unique_ptr<key::style::MIRStyle> style(key::style::MIRStyleFactory::build(param));

    param::RuntimeParametrisation runtime(param::DefaultParametrisation::instance());
    action::ActionPlan plan(runtime);
    output::EmptyOutput output;
    EXPECT_THROWS(style->prepare(plan, output));
}


}  // namespace mir::action::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
