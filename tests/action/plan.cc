// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "eckit/filesystem/TmpFile.h"
#include "eckit/parser/YAMLParser.h"
#include "eckit/value/Value.h"

#include "mir/action/io/Save.h"
#include "mir/action/plan/ActionPlan.h"
#include "mir/action/plan/Job.h"
#include "mir/api/MIRJob.h"
#include "mir/input/MIRInput.h"
#include "mir/output/EmptyOutput.h"
#include "mir/param/DefaultParametrisation.h"
#include "mir/param/RuntimeParametrisation.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/util/BoundingBox.h"

#include "action.h"


namespace mir::action::tests {


const std::string O32 = "{artificialInput: gridspec, gridSpec: {grid: O32}}";


std::unique_ptr<input::MIRInput> artificial(const std::string& input) {
    param::SimpleParametrisation param;
    param.set("input", input);

    std::unique_ptr<input::MIRInput> in(input::MIRInputFactory::build("constant", param));
    ASSERT(in->next());
    return in;
}


std::vector<std::string> types(const ActionPlan& plan) {
    std::vector<std::string> types;
    for (const auto* action : plan) {
        types.emplace_back(action->type());
    }
    return types;
}


std::string read(const eckit::PathName& path) {
    std::ifstream in(path.asString());
    std::ostringstream str;
    str << in.rdbuf();
    return str.str();
}


bool contains(const std::string& str, const std::string& sub) {
    return str.find(sub) != std::string::npos;
}


CASE("ActionPlan, adding actions") {
    param::RuntimeParametrisation param(param::DefaultParametrisation::instance());
    param.set("area", "60/-10/30/40");

    output::EmptyOutput output;
    ActionPlan plan(param);

    EXPECT(plan.empty());
    EXPECT(!plan.ended());

    // by name, with (runtime) options of different types
    plan.add("crop.area");
    plan.add("filter.sh-truncate", "truncation", 21L);
    plan.add("set.metadata", "metadata", "paramId=1");
    plan.add("filter.statistics", "statistics", "min-max", "precision", 3L);
    plan.add("calc.formula", "formula", "f + 1", "formula.metadata", "paramId=2");

    // by partial name, with a parametrisation owned by the plan
    auto* frame = new param::RuntimeParametrisation(static_cast<const param::MIRParametrisation&>(param));
    frame->set("frame", 2L);
    plan.add("frame", frame);

    EXPECT(!plan.ended());

    plan.add(new io::Save(param, output));

    EXPECT(plan.ended());
    EXPECT_THROWS(plan.add("noop"));  // the plan has ended

    const std::vector<std::string> expected{"crop.area",    "filter.sh-truncate", "set.metadata", "filter.statistics",
                                            "calc.formula", "filter.frame",       "save"};
    EXPECT(types(plan) == expected);

    const auto& p = plan;
    EXPECT_EQUAL(p.action(0).json_str(), R"({"type":"crop.area","area":[60,-10,30,40]})");
    EXPECT_EQUAL(p.action(1).json_str(), R"({"type":"filter.sh-truncate","truncation":21})");
    EXPECT_EQUAL(p.action(3).json_str(), R"({"type":"filter.statistics","statistics":"min-max","precision":3})");
    EXPECT_EQUAL(p.action(5).json_str(), R"({"type":"filter.frame","frame":2})");

    SECTION("json") {
        const auto json = plan.json_str();

        EXPECT(json.front() == '[' && json.back() == ']');
        EXPECT(json.rfind(R"([{"type":"crop.area","area":[60,-10,30,40]},{"type":"filter.sh-truncate")", 0) == 0);
        EXPECT(contains(json, R"(,{"type":"filter.frame","frame":2},{"type":"save"}])"));
    }

    SECTION("printing") {
        std::ostringstream print;
        print << plan;

        EXPECT(print.str().rfind("ActionPlan[AreaCropper[bbox=", 0) == 0);
        EXPECT(contains(print.str(), " ==> FrameFilter[size=2] ==> Save[output=EmptyOutput[]]]"));
    }

    SECTION("dump, a line per action") {
        std::ostringstream dump;
        plan.dump(dump);

        std::istringstream lines(dump.str());
        size_t n = 0;
        for (std::string line; std::getline(lines, line); ++n) {
            EXPECT(line.rfind("      ==> ", 0) == 0);
        }

        EXPECT_EQUAL(n, plan.size());
    }

    SECTION("custom dump, actions separated by |") {
        std::ostringstream custom;
        plan.custom(custom);

        EXPECT(custom.str().rfind("AreaCropper[bbox=", 0) == 0);
        EXPECT(contains(custom.str(), "|FrameFilter[size=2]|Save[output=...]"));
    }
}


CASE("ActionPlan, known actions spec") {
    const param::RuntimeParametrisation param(param::DefaultParametrisation::instance());
    output::EmptyOutput output;

    // known actions are by name
    ActionPlan plan(param);
    plan.add("filter.uv-vorticity");
    plan.add(new io::Save(param, output));

    EXPECT_EQUAL(plan.json_str(), R"(["filter.uv-vorticity",{"type":"save"}])");
}


CASE("ActionPlan, compress") {
    param::RuntimeParametrisation param(param::DefaultParametrisation::instance());
    output::EmptyOutput output;

    ActionPlan plan(param);

    SECTION("interpolation merged with cropping") {
        plan.add("noop");
        plan.add("interpolate.grid2regular-ll", "grid", "2/2");
        plan.add("crop.area", "area", "60/-10/30/40");
        plan.add(new io::Save(param, output));

        plan.compress();

        EXPECT(types(plan) == std::vector<std::string>({"noop", "interpolate.grid2regular-ll", "save"}));
        const auto& interpolation = static_cast<const ActionPlan&>(plan).action(1);
        EXPECT(interpolation.canCrop());
        EXPECT(interpolation.outputBoundingBox() == util::BoundingBox(60, -10, 30, 40));
    }

    SECTION("globalisation deleted before cropping") {
        plan.add("filter.globalise");
        plan.add("crop.area", "area", "60/-10/30/40");
        plan.add(new io::Save(param, output));

        plan.compress();

        EXPECT(types(plan) == std::vector<std::string>({"crop.area", "save"}));
    }

    SECTION("nothing to compress") {
        plan.add("noop");
        plan.add("filter.globalise");
        plan.add(new io::Save(param, output));

        plan.compress();

        EXPECT(types(plan) == std::vector<std::string>({"noop", "filter.globalise", "save"}));
    }
}


CASE("Job, plan") {
    auto input = artificial(O32);
    output::EmptyOutput output;
    api::MIRJob job;

    SECTION("no post-processing: copy") {
        const Job copy(job, *input, output, true);
        EXPECT(types(copy.plan()) == std::vector<std::string>{"copy"});

        util::MIRStatistics statistics;
        EXPECT_NO_THROW(copy.execute(statistics));
    }

    SECTION("the input grid: copy") {
        const Job copy(job.set("grid", "O32"), *input, output, true);
        EXPECT(types(copy.plan()) == std::vector<std::string>{"copy"});
    }

    SECTION("compressed, or not") {
        job.set("grid", std::vector<double>{10, 10}).set("area", std::vector<double>{60, -10, 30, 40});

        const Job compressed(job, *input, output, true);
        const Job uncompressed(job, *input, output, false);

        EXPECT(types(compressed.plan()) == std::vector<std::string>({"interpolate.grid2regular-ll", "save"}));
        EXPECT(types(uncompressed.plan()) ==
               std::vector<std::string>({"interpolate.grid2regular-ll", "crop.area", "save"}));
    }
}


CASE("Job, execute") {
    auto input = artificial(O32);
    output::EmptyOutput output;
    util::MIRStatistics statistics;

    eckit::TmpFile plan;
    eckit::TmpFile actions;
    eckit::TmpFile grids;
    eckit::TmpFile stats;

    api::MIRJob job;
    job.set("grid", std::vector<double>{10, 10});
    job.set("dump-action-plan-file", actions.asString()).set("dump-grid-plan-file", grids.asString());

    SECTION("with dumps") {
        job.set("dump-plan-file", plan.asString()).set("dump-statistics-file", stats.asString());

        const Job j(job, *input, output, true);
        j.execute(statistics);

        // the plan (custom), the actions (json) and the grids (json): the input, then after each action
        EXPECT(read(plan).rfind("Gridded2RegularLL[", 0) == 0);
        EXPECT(contains(read(plan), "|Save[output=...]\n"));

        EXPECT_EQUAL(read(actions), j.plan().json_str() + "\n");

        const auto g = eckit::YAMLParser::decodeString(read(grids));
        EXPECT(g.isList());
        EXPECT_EQUAL(g.size(), j.plan().size() + 1);
        EXPECT_EQUAL(std::string(g[0]["grid"]), "O32");

        EXPECT(read(stats).front() == '{');
    }

    SECTION("dry run") {
        const Job j(job.set("dryrun", true), *input, output, true);
        j.execute(statistics);

        // the actions are dumped, the grids are not (the plan is not executed)
        EXPECT_EQUAL(read(actions), j.plan().json_str() + "\n");
        EXPECT(read(grids).empty());
    }

    SECTION("grid plan") {
        const Job j(job, *input, output, true);
        const auto g = eckit::YAMLParser::decodeString(j.executeGridPlan(statistics));

        EXPECT(g.isList());
        EXPECT_EQUAL(g.size(), j.plan().size() + 1);
        EXPECT_EQUAL(std::string(g[0]["grid"]), "O32");

        // the grid plan is not dumped
        EXPECT(read(grids).empty());
    }
}


}  // namespace mir::action::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
