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


#ifndef Gridded2ReducedGG_H
#define Gridded2ReducedGG_H

#include "mir/action/interpolate/Gridded2GriddedInterpolation.h"

namespace mir {
namespace action {


class Gridded2ReducedGG : public Gridded2GriddedInterpolation {
  public:

// -- Exceptions
    // None

// -- Contructors

    Gridded2ReducedGG(const param::MIRParametrisation&);

// -- Destructor

    virtual ~Gridded2ReducedGG(); // Change to virtual if base class

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

    Gridded2ReducedGG(const Gridded2ReducedGG&);
    Gridded2ReducedGG& operator=(const Gridded2ReducedGG&);

// -- Members

    size_t N_;

// -- Methods
    // None

// -- Overridden methods

    // From Gridded2GriddedInterpolation
    virtual repres::Representation* outputRepresentation(const repres::Representation* inputRepres) const;

// -- Class members
    // None

// -- Class methods
    // None

// -- Friends

    //friend ostream& operator<<(ostream& s,const Gridded2ReducedGG& p)
    //	{ p.print(s); return s; }

};


}  // namespace action
}  // namespace mir
#endif

