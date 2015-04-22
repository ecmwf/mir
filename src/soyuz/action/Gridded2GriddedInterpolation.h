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


#ifndef Gridded2GriddedInterpolation_H
#define Gridded2GriddedInterpolation_H

#include "soyuz/action/Action.h"


namespace mir {
namespace action {


class Gridded2GriddedInterpolation : public Action {
  public:

// -- Exceptions
    // None

// -- Contructors

    Gridded2GriddedInterpolation(const MIRParametrisation&);

// -- Destructor

    virtual ~Gridded2GriddedInterpolation(); // Change to virtual if base class

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

    Gridded2GriddedInterpolation(const Gridded2GriddedInterpolation&);
    Gridded2GriddedInterpolation& operator=(const Gridded2GriddedInterpolation&);

// -- Members
    // None

// -- Methods
    // None

// -- Overridden methods

    virtual void execute(data::MIRField&) const;

// -- Class members
    // None

// -- Class methods
    // None

// -- Friends

    //friend ostream& operator<<(ostream& s,const Gridded2GriddedInterpolation& p)
    //	{ p.print(s); return s; }

};


}  // namespace action
}  // namespace mir
#endif

