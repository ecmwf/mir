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


#ifndef ActionNode_H
#define ActionNode_H

#include <iosfwd>

#include "mir/action/plan/ActionGraph.h"

namespace mir {

namespace data {
class Context;
}


namespace util {
class MIRStatistics;
}


namespace api {
class MIRWatcher;
}

namespace action {

class Action;

class ActionNode {
  public:

// -- Exceptions
    // None

// -- Contructors

    ActionNode(const Action& action, api::MIRWatcher *watcher);

// -- Destructor

    ~ActionNode(); // Change to virtual if base class

// -- Convertors
    // None

// -- Operators
    // None

// -- Methods

    void execute(context::Context& ctx, const Executor& executor) const;

    //=====================================

    const action::Action &action() const;

    ActionGraph& graph();

    void dump(std::ostream& out, size_t depth) const;

    void notifyFailure(std::exception&, const Action& action, api::MIRWatcher *watcher, bool& rethrow) const;

// -- Overridden methods
    // None

// -- Class members
    // None

// -- Class methods
    // None

  protected:

// -- Members


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

    ActionNode(const ActionNode&);
    ActionNode& operator=(const ActionNode&);

// -- Members

    const Action &action_;
    ActionGraph graph_;
    api::MIRWatcher* watcher_;  // Just a reference, do not won

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


}  // namespace action
}  // namespace mir
#endif

