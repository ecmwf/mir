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


#include <memory>

#include "eckit/testing/Test.h"

#include "mir/action/area/AreaCropper.h"
#include "mir/action/plan/Action.h"
#include "mir/action/plan/ActionGraph.h"
#include "mir/action/plan/ActionNode.h"
#include "mir/action/plan/ActionPlan.h"
#include "mir/api/MIRWatcher.h"
#include "mir/param/DefaultParametrisation.h"
#include "mir/param/RuntimeParametrisation.h"
#include "mir/util/Log.h"

// define EXPECTV(a) log << "\tEXPECT(" << #a <<")" << std::endl; EXPECT(a)


namespace mir::tests::unit {


struct TestWatcher : api::MIRWatcher {
    void print(std::ostream& /*unused*/) const override {}
    bool failure(std::exception& e, const mir::action::Action& action) override {
        Log::error() << "Exception: '" << e.what() << "' on " << action << std::endl;
        throw;
    }
};


CASE("ActionGraph") {

    std::unique_ptr<api::MIRWatcher> watcher(new TestWatcher());
    param::DefaultParametrisation empty;


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
}


}  // namespace mir::tests::unit


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
