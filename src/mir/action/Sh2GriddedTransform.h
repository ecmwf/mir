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


#ifndef Sh2GriddedTransform_H
#define Sh2GriddedTransform_H

#include "mir/action/Action.h"



namespace mir {

namespace repres {
class Representation;
}

namespace action {


class Sh2GriddedTransform : public Action {
  public:

// -- Exceptions
    // None

// -- Contructors

    Sh2GriddedTransform(const param::MIRParametrisation&);

// -- Destructor

    virtual ~Sh2GriddedTransform(); // Change to virtual if base class

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

    Sh2GriddedTransform(const Sh2GriddedTransform&);
    Sh2GriddedTransform& operator=(const Sh2GriddedTransform&);

// -- Members
    // None

// -- Methods

    virtual repres::Representation* outputRepresentation(const repres::Representation* inputRepres) const = 0;

// -- Overridden methods

    virtual void execute(data::MIRField&) const;

// -- Class members
    // None

// -- Class methods
    // None

// -- Friends

    //friend ostream& operator<<(ostream& s,const Sh2GriddedTransform& p)
    //	{ p.print(s); return s; }

};


}  // namespace action
}  // namespace mir
#endif

