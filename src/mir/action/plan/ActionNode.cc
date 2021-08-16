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


#include <ostream>

#include "mir/action/plan/Action.h"
#include "mir/action/plan/ActionNode.h"
#include "mir/action/plan/Executor.h"
#include "mir/util/Log.h"


namespace mir {
namespace action {


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


}  // namespace action
}  // namespace mir
