/*
 * (C) Copyright 1996-2015 ECMWF.
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


#ifndef mir_action_ActionPlan_h
#define mir_action_ActionPlan_h

#include <string>
#include <vector>
#include "mir/param/MIRParametrisation.h"


namespace mir {
namespace context {
class Context;
}
namespace action {
class Action;
}
}


namespace mir {
namespace action {
class ActionPlan {
public:

    // -- Exceptions
    // None

    // -- Contructors

    ActionPlan(const param::MIRParametrisation& parametrisation);

    // -- Destructor

    ~ActionPlan(); // Change to virtual if base class

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    void add(const std::string& name);
    void add(const std::string& name, const std::string&, long);
    void add(const std::string& name, const std::string&, const std::string&);
    void add(const std::string& name, const std::string&, const std::string&, const std::string&, long);
    void add(const std::string& name, const std::string&, const std::string&, const std::string&, const std::string&);

    void add(Action* action);
    void add(const std::string &name, param::MIRParametrisation* runtime);


    void execute(context::Context& ctx) const;
    bool empty() const;
    size_t size() const;
    const Action& action(size_t) const;

    void dump(std::ostream&) const;
    void custom(std::ostream&) const;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

protected:

    // -- Members

    const param::MIRParametrisation& parametrisation_;
    std::vector<Action*> actions_;
    std::vector<param::MIRParametrisation*> runtimes_;

    // -- Methods

    void print(std::ostream&) const;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:

    // No copy allowed
    ActionPlan(const ActionPlan&);
    ActionPlan& operator=(const ActionPlan&);

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

    friend std::ostream& operator<<(std::ostream& s, const ActionPlan& p) {
        p.print(s);
        return s;
    }

};


}  // namespace action
}  // namespace mir


#endif

