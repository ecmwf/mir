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


#ifndef Gridded2NamedGrid_H
#define Gridded2NamedGrid_H

#include "mir/action/interpolate/Gridded2GriddedInterpolation.h"

namespace mir {
namespace action {


class Gridded2NamedGrid : public Gridded2GriddedInterpolation {
  public:

// -- Exceptions
    // None

// -- Contructors

    Gridded2NamedGrid(const param::MIRParametrisation&);

// -- Destructor

    virtual ~Gridded2NamedGrid(); // Change to virtual if base class

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

    Gridded2NamedGrid(const Gridded2NamedGrid&);
    Gridded2NamedGrid& operator=(const Gridded2NamedGrid&);

// -- Members

    std::string gridname_;

// -- Methods
    // None

// -- Overridden methods

    // From Gridded2GriddedInterpolation
    virtual const repres::Representation* outputRepresentation() const;

// -- Class members
    // None

// -- Class methods
    // None

// -- Friends

    //friend ostream& operator<<(ostream& s,const Gridded2NamedGrid& p)
    //	{ p.print(s); return s; }

};


}  // namespace action
}  // namespace mir
#endif

