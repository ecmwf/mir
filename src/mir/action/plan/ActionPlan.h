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


#ifndef mir_action_plan_ActionPlan_h
#define mir_action_plan_ActionPlan_h

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


class ActionPlan : protected std::vector<Action*> {
private:

    // -- Types

    using container_t = std::vector<Action*>;

public:

    // -- Exceptions
    // None

    // -- Contructors

    ActionPlan(const param::MIRParametrisation&);

    // -- Destructor

    ~ActionPlan();

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

    void add(Action*);
    void add(const std::string& name, param::MIRParametrisation*);

    void execute(context::Context& ctx) const;
    void dump(std::ostream&) const;
    void custom(std::ostream&) const;
    void compress();

    const Action& action(size_t) const;

    using container_t::back;
    using container_t::begin;
    using container_t::cbegin;
    using container_t::cend;
    using container_t::crbegin;
    using container_t::crend;
    using container_t::empty;
    using container_t::end;
    using container_t::front;
    using container_t::operator[];
    using container_t::rbegin;
    using container_t::rend;
    using container_t::size;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

protected:

    // -- Members

    const param::MIRParametrisation& parametrisation_;
    std::vector<param::MIRParametrisation*> runtimes_;

    // -- Methods

    void print(std::ostream&) const;
    Action& action(size_t);

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

