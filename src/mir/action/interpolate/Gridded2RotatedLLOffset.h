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


#ifndef Gridded2RotatedLLOffset_H
#define Gridded2RotatedLLOffset_H

#include "mir/action/interpolate/Gridded2GriddedInterpolation.h"
#include "mir/util/Increments.h"
#include "mir/util/Rotation.h"


namespace mir {
namespace action {


class Gridded2RotatedLLOffset : public Gridded2GriddedInterpolation {
public:

    // -- Exceptions
    // None

    // -- Contructors

    Gridded2RotatedLLOffset(const param::MIRParametrisation&);

    // -- Destructor

    virtual ~Gridded2RotatedLLOffset(); // Change to virtual if base class

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

    Gridded2RotatedLLOffset(const Gridded2RotatedLLOffset&);
    Gridded2RotatedLLOffset& operator=(const Gridded2RotatedLLOffset&);

    // -- Members

    util::Increments increments_;
    util::Rotation rotation_;
    double northwards_;
    double eastwards_;

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

    //friend ostream& operator<<(ostream& s,const Gridded2RotatedLLOffset& p)
    //  { p.print(s); return s; }

};


}  // namespace action
}  // namespace mir


#endif

