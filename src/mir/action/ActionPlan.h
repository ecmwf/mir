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


#ifndef ActionPlan_H
#define ActionPlan_H

#include <string>


#include "mir/param/RuntimeParametrisation.h"

namespace mir {

namespace data {
class MIRField;
}

namespace action {

class Action;

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

    void execute(data::MIRField&) const;

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

    void print(std::ostream&) const; // Change to virtual if base class

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

