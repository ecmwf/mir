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


#ifndef Gridded2RegularLL_H
#define Gridded2RegularLL_H

#include "mir/action/interpolate/Gridded2LatLon.h"


namespace mir {
namespace action {


class Gridded2RegularLL : public Gridded2LatLon {
public:

    // -- Exceptions
    // None

    // -- Contructors

    Gridded2RegularLL(const param::MIRParametrisation&);

    // -- Destructor

    virtual ~Gridded2RegularLL(); // Change to virtual if base class

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

    Gridded2RegularLL(const Gridded2RegularLL&);
    Gridded2RegularLL& operator=(const Gridded2RegularLL&);

    // -- Members

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

    //friend ostream& operator<<(ostream& s,const Gridded2RegularLL& p)
    //	{ p.print(s); return s; }

};


}  // namespace action
}  // namespace mir


#endif

