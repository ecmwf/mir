// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <iosfwd>

#include "mir/action/plan/ActionGraph.h"


namespace mir {
namespace api {
class MIRWatcher;
}
namespace action {
class Action;
}
}  // namespace mir


namespace mir::action {


class ActionNode {
public:
    // -- Exceptions
    // None

    // -- Constructors

    ActionNode(const Action&, api::MIRWatcher*);

    ActionNode(const ActionNode&) = delete;
    ActionNode(ActionNode&&)      = delete;

    // -- Destructor

    ~ActionNode();

    // -- Convertors
    // None

    // -- Operators

    void operator=(const ActionNode&) = delete;
    void operator=(ActionNode&&)      = delete;

    // -- Methods

    void execute(context::Context&, const Executor&) const;

    const Action& action() const { return action_; }

    ActionGraph& graph() { return graph_; }

    void dump(std::ostream&, size_t depth) const;

    void notifyFailure(std::exception&, const Action&, api::MIRWatcher*, bool& rethrow) const;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

protected:
    // -- Members
    // None

    // -- Methods

    void print(std::ostream&) const;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Members

    const Action& action_;
    ActionGraph graph_;
    api::MIRWatcher* watcher_;  // Just a reference, do not own

    // -- Methods
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    friend std::ostream& operator<<(std::ostream& s, const ActionNode& p) {
        p.print(s);
        return s;
    }
};


}  // namespace mir::action
