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


#ifndef NullAction_H
#define NullAction_H

#include "soyuz/action/Action.h"


namespace mir {
namespace action {


class NullAction : public Action {
  public:

// -- Exceptions
    // None

// -- Contructors

    NullAction(const MIRParametrisation&);

// -- Destructor

    virtual ~NullAction(); // Change to virtual if base class

// -- Convertors
    // None

// -- Operators
    // None

// -- Methods
    // None

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

    NullAction(const NullAction&);
    NullAction& operator=(const NullAction&);

// -- Members
    // None

// -- Methods
    // None

// -- Overridden methods

    virtual void execute(MIRField&) const;

// -- Class members
    // None

// -- Class methods
    // None

// -- Friends

    //friend ostream& operator<<(ostream& s,const NullAction& p)
    //	{ p.print(s); return s; }

};


}  // namespace action
}  // namespace mir
#endif

