// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/action/plan/ActionGraph.h"

#include <ostream>

#include "mir/action/context/Context.h"
#include "mir/action/plan/Action.h"
#include "mir/action/plan/ActionNode.h"
#include "mir/action/plan/ActionPlan.h"
#include "mir/action/plan/Executor.h"
#include "mir/api/MIRWatcher.h"


namespace mir::action {


ActionGraph::ActionGraph() = default;


ActionGraph::~ActionGraph() {
    for (auto& j : *this) {
        delete j;
    }
}


void ActionGraph::execute(context::Context& ctx, const Executor& executor) const {
    for (const auto& j : *this) {
        context::Context local(ctx);
        executor.execute(local, *j);
    }
}


void ActionGraph::add(const ActionPlan& plan, api::MIRWatcher* watcher) {
    ActionGraph* current = this;

    size_t i = 0;
    while (i < plan.size()) {
        bool found = false;

        for (auto& k : *current) {
            if (plan.action(i).sameAs(k->action())) {
                current = &k->graph();
                found   = true;
                break;
            }
        }

        if (!found) {
            break;
        }
        i++;
    }

    while (i < plan.size()) {
        ActionNode* node = current->add(plan.action(i), watcher);
        current          = &node->graph();
        i++;
    }
}


ActionNode* ActionGraph::add(const Action& action, api::MIRWatcher* watcher) {
    push_back(new ActionNode(action, watcher));
    return back();
}


void ActionGraph::dump(std::ostream& out, size_t depth) const {
    for (const auto& j : *this) {
        j->dump(out, depth);
    }
}


void ActionGraph::print(std::ostream& out) const {
    out << "ActionGraph[\n";
    dump(out, 1);
    out << "]";
}


void ActionGraph::notifyFailure(std::exception& e, const Action& action, api::MIRWatcher* watcher,
                                bool& rethrow) const {
    if (empty()) {
        if (watcher != nullptr) {
            rethrow = watcher->failure(e, action) && rethrow;
        }
    }
    for (const auto& j : *this) {
        j->notifyFailure(e, action, watcher, rethrow);
    }
}


}  // namespace mir::action
