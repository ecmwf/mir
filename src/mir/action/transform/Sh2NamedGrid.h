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


#ifndef Sh2NamedGrid_H
#define Sh2NamedGrid_H

#include "mir/action/transform/Sh2GriddedTransform.h"

namespace mir {
namespace action {
namespace transform {


class Sh2NamedGrid : public Sh2GriddedTransform {
  public:

// -- Exceptions
    // None

// -- Contructors

    Sh2NamedGrid(const param::MIRParametrisation&);

// -- Destructor

    virtual ~Sh2NamedGrid(); // Change to virtual if base class

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

    Sh2NamedGrid(const Sh2NamedGrid&);
    Sh2NamedGrid& operator=(const Sh2NamedGrid&);

// -- Members

    std::string gridname_;

// -- Methods
    // None

// -- Overridden methods

    virtual bool sameAs(const Action& other) const;

    // From Gridded2GriddedInterpolation
    virtual const repres::Representation* outputRepresentation() const;

// -- Class members
    // None

// -- Class methods
    // None

// -- Friends

    //friend ostream& operator<<(ostream& s,const Sh2NamedGrid& p)
    //	{ p.print(s); return s; }

};


}  // namespace transform
}  // namespace action
}  // namespace mir


#endif

