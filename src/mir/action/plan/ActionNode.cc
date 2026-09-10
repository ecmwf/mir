// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/action/plan/ActionNode.h"

#include <ostream>

#include "mir/action/plan/Action.h"
#include "mir/action/plan/Executor.h"
#include "mir/util/Log.h"


namespace mir::action {


ActionNode::ActionNode(const Action& action, api::MIRWatcher* watcher) : action_(action), watcher_(watcher) {}


ActionNode::~ActionNode() = default;


void ActionNode::print(std::ostream& out) const {
    out << "ActionNode[" << action_ << "]";
}


void ActionNode::execute(context::Context& ctx, const Executor& executor) const {
    // Log::info() << " BEFORE -----> " << action_  << "  " << field << std::endl;
    bool ok = false;
    try {
        action_.perform(ctx);
        ok = true;
    }
    catch (std::exception& e) {

        Log::error() << e.what() << " while executing " << action_ << std::endl;

        bool rethrow = true;
        notifyFailure(e, action_, watcher_, rethrow);
        if (rethrow) {
            throw;
        }
    }
    // Log::info() << " AFTER -----> " << action_  << "  " << field << std::endl;

    if (ok) {
        graph_.execute(ctx, executor);
    }
}


void ActionNode::notifyFailure(std::exception& e, const Action& action, api::MIRWatcher* /*watcher*/,
                               bool& rethrow) const {
    graph_.notifyFailure(e, action, watcher_, rethrow);
}


void ActionNode::dump(std::ostream& out, size_t depth) const {
    for (size_t i = 0; i < depth; i++) {
        out << "   ";
    }
    out << action_ << std::endl;
    graph_.dump(out, depth + 1);
}


}  // namespace mir::action
