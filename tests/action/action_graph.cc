// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "eckit/filesystem/TmpFile.h"
#include "eckit/log/JSON.h"
#include "eckit/parser/YAMLParser.h"
#include "eckit/testing/Test.h"
#include "eckit/value/Value.h"

#include "mir/action/area/AreaCropper.h"
#include "mir/action/plan/Action.h"
#include "mir/action/plan/ActionGraph.h"
#include "mir/action/plan/ActionNode.h"
#include "mir/action/plan/ActionPlan.h"
#include "mir/api/MIRComplexJob.h"
#include "mir/api/MIRJob.h"
#include "mir/api/MIRWatcher.h"
#include "mir/input/GribFileInput.h"
#include "mir/output/EmptyOutput.h"
#include "mir/param/DefaultParametrisation.h"
#include "mir/param/RuntimeParametrisation.h"
#include "mir/util/Log.h"
#include "mir/util/MIRStatistics.h"

// define EXPECTV(a) log << "\tEXPECT(" << #a <<")" << std::endl; EXPECT(a)


namespace mir::action::tests {


struct TestWatcher : api::MIRWatcher {
    void print(std::ostream& /*unused*/) const override {}
    bool failure(std::exception& e, const mir::action::Action& action) override {
        Log::error() << "Exception: '" << e.what() << "' on " << action << std::endl;
        throw;
    }
};


CASE("ActionGraph") {

    std::unique_ptr<api::MIRWatcher> watcher(new TestWatcher());
    const auto& empty = param::DefaultParametrisation::instance();


    param::RuntimeParametrisation area1(empty);
    param::RuntimeParametrisation area2(empty);
    area1.set("area", "0/0/0/10");
    area2.set("area", "0/360/0/370");

    action::ActionPlan plan1(area1);
    action::ActionPlan plan2(area2);
    plan1.add("crop.area");
    plan2.add("crop.area");

    std::unique_ptr<action::Action> reference1(new action::AreaCropper(area1));
    std::unique_ptr<action::Action> reference2(new action::AreaCropper(area2));


    SECTION("ActionGraph with two unique actions (variant 1/2)") {
        action::ActionGraph graph;
        graph.add(plan1, watcher.get());
        graph.add(plan2, watcher.get());

        Log::debug() << graph << std::endl;
        EXPECT(graph.size() == 2);
        EXPECT(graph[0]->action().sameAs(*reference1));
        EXPECT(graph[1]->action().sameAs(*reference2));
    }


    SECTION("ActionGraph with two unique actions (variant 2/2)") {
        action::ActionGraph graph;
        graph.add(plan2, watcher.get());
        graph.add(plan1, watcher.get());

        Log::debug() << graph << std::endl;
        EXPECT(graph.size() == 2);
        EXPECT(graph[0]->action().sameAs(*reference2));
        EXPECT(graph[1]->action().sameAs(*reference1));
    }


    SECTION("ActionGraph with two similar actions (variant 1/2)") {
        action::ActionGraph graph;
        graph.add(plan1, watcher.get());
        graph.add(plan1, watcher.get());

        Log::debug() << graph << std::endl;
        EXPECT(graph.size() == 1);
        EXPECT(graph[0]->action().sameAs(*reference1));
    }


    SECTION("ActionGraph with two similar actions (variant 2/2)") {
        action::ActionGraph graph;
        graph.add(plan2, watcher.get());
        graph.add(plan2, watcher.get());

        Log::debug() << graph << std::endl;
        EXPECT(graph.size() == 1);
        EXPECT(graph[0]->action().sameAs(*reference2));
    }


    SECTION("ActionGraph with two similar data-dependent actions") {
        // linear interpolation is merged, statistics-based interpolation is not (not the same, see Method::sameAs)
        for (const auto& test :
             std::vector<std::pair<std::string, size_t>>{{"grid-box-average", 1}, {"grid-box-statistics", 2}}) {
            const auto size = test.second;

            param::RuntimeParametrisation param(empty);
            param.set("grid", std::vector<double>{1., 1.});
            param.set("interpolation", test.first);

            action::ActionPlan plan(param);
            plan.add("interpolate.grid2regular-ll");

            action::ActionGraph graph;
            graph.add(plan, watcher.get());
            graph.add(plan, watcher.get());

            Log::debug() << graph << std::endl;
            EXPECT_EQUAL(graph.size(), size);
            EXPECT_EQUAL(graph[0]->action().json_str(), graph[size - 1]->action().json_str());
        }
    }
}


CASE("ActionGraph JSON") {
    const auto& empty = param::DefaultParametrisation::instance();
    param::RuntimeParametrisation area(empty);
    area.set("area", "0/0/0/10");

    action::ActionPlan plan1(area);
    plan1.add("crop.area");
    plan1.add("noop");

    action::ActionPlan plan2(area);
    plan2.add("crop.area");
    plan2.add("mask.area");

    // a tree, of the common action and the following
    action::ActionGraph graph;
    graph.add(plan1, nullptr);
    graph.add(plan2, nullptr);

    EXPECT_EQUAL(plan1.json_str(), R"([{"type":"crop.area","area":[0,0,0,10]},{"type":"noop"}])");

    std::ostringstream out;
    eckit::JSON j(out);
    graph.json(j);
    EXPECT_EQUAL(out.str(), R"([[{"type":"crop.area","area":[0,0,0,10]},[[{"type":"noop"},[]],)"
                            R"([{"type":"mask.area","area":[0,0,0,10]},[]]]]])");
}


CASE("MIRComplexJob dumps") {
    const eckit::TmpFile actions;
    const eckit::TmpFile grids;

    input::GribFileInput input("MIR-425.grib1");
    ASSERT(input.next());

    output::EmptyOutput output1;
    output::EmptyOutput output2;

    // dump options of the first job
    auto* job1 = new api::MIRJob;
    job1->set("grid", 2., 2.).set("dump-action-plan-file", actions.asString());
    job1->set("dump-grid-plan-file", grids.asString());

    auto* job2 = new api::MIRJob;
    job2->set("grid", 2., 2.).set("area", 10., 0., 0., 10.);

    api::MIRComplexJob complex;
    complex.add(job1, input, output1).add(job2, input, output2);

    util::MIRStatistics statistics;
    complex.execute(statistics);

    // trees: two (regrid) roots, each followed by its own actions
    auto read = [](const eckit::PathName& path) {
        std::ifstream in(path.asString());
        std::string line;
        std::getline(in, line);
        return eckit::YAMLParser::decodeString(line);
    };

    const auto a = read(actions);
    EXPECT(a.isList() && a.size() == 2);

    // grids: the input grid, then the same tree shape
    const auto g = read(grids);
    EXPECT(g.isList() && g.size() == 2);
    EXPECT(g[1].isList() && g[1].size() == a.size());
    EXPECT(g[1][0][1].size() == a[0][1].size());
}


}  // namespace mir::action::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
