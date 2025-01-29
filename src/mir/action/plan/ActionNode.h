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
