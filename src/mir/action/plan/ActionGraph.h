// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once


#include <iosfwd>
#include <vector>


namespace mir {
namespace context {
class Context;
}
namespace action {
class Action;
class ActionNode;
class ActionPlan;
class Executor;
}  // namespace action
namespace api {
class MIRWatcher;
}
}  // namespace mir


namespace mir::action {


class ActionGraph : protected std::vector<ActionNode*> {
private:
    using vector_t = std::vector<ActionNode*>;

public:
    // -- Exceptions
    // None

    // -- Constructors

    ActionGraph();

    ActionGraph(const ActionGraph&) = delete;
    ActionGraph(ActionGraph&&)      = delete;

    // -- Destructor

    ~ActionGraph();

    // -- Convertors
    // None

    // -- Operators

    void operator=(const ActionGraph&) = delete;
    void operator=(ActionGraph&&)      = delete;

    // -- Methods

    using vector_t::empty;
    using vector_t::size;
    using vector_t::operator[];

    void execute(context::Context&, const Executor&) const;

    void add(const ActionPlan&, api::MIRWatcher*);

    ActionNode* add(const Action&, api::MIRWatcher*);

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
    // None

    // -- Methods
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    friend std::ostream& operator<<(std::ostream& s, const ActionGraph& p) {
        p.print(s);
        return s;
    }
};


}  // namespace mir::action
