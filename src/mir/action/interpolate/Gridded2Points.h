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


#ifndef Gridded2Points_H
#define Gridded2Points_H

#include "mir/action/interpolate/Gridded2GriddedInterpolation.h"

#include <vector>

namespace mir {
namespace action {


class Gridded2Points : public Gridded2GriddedInterpolation {
  public:

// -- Exceptions
    // None

// -- Contructors

    Gridded2Points(const param::MIRParametrisation&);

// -- Destructor

    virtual ~Gridded2Points(); // Change to virtual if base class

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

    Gridded2Points(const Gridded2Points&);
    Gridded2Points& operator=(const Gridded2Points&);

// -- Members

    std::vector<double> latitudes_;
    std::vector<double> longitudes_;

// -- Methods
    // None

// -- Overridden methods

    virtual bool sameAs(const Action& other) const;
    virtual const char* name() const;

    // From Gridded2GriddedInterpolation
    virtual const repres::Representation* outputRepresentation() const;

// -- Class members
    // None

// -- Class methods
    // None

// -- Friends

    //friend ostream& operator<<(ostream& s,const Gridded2Points& p)
    //	{ p.print(s); return s; }

};


}  // namespace action
}  // namespace mir
#endif

