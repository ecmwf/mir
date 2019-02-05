/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date Apr 2015


#ifndef mir_action_plan_ActionGraph_H
#define mir_action_plan_ActionGraph_H


#include <vector>
#include <iosfwd>


namespace mir {
namespace context {
class Context;
}
namespace action {
class Action;
class ActionNode;
class ActionPlan;
class Executor;
}
namespace api {
class MIRWatcher;
}
}


namespace mir {
namespace action {


class ActionGraph : protected std::vector<ActionNode*> {
private:

    using vector_t = std::vector<ActionNode*>;

public:

    // -- Exceptions
    // None

    // -- Contructors

    ActionGraph();

    // -- Destructor

    ~ActionGraph(); // Change to virtual if base class

    // -- Convertors
    // None

    // -- Operators
    // None

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

    void print(std::ostream&) const; // Change to virtual if base class

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:

    // No copy allowed
    ActionGraph(const ActionGraph&);
    ActionGraph& operator=(const ActionGraph&);

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


}  // namespace action
}  // namespace mir


#endif

