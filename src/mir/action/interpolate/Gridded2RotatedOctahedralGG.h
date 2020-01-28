/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#ifndef mir_action_interpolate_Gridded2RotatedOctahedralGG_h
#define mir_action_interpolate_Gridded2RotatedOctahedralGG_h

#include "mir/action/interpolate/Gridded2RotatedGrid.h"


namespace mir {
namespace action {
namespace interpolate {


class Gridded2RotatedOctahedralGG : public Gridded2RotatedGrid {
public:
    // -- Exceptions
    // None

    // -- Contructors

    Gridded2RotatedOctahedralGG(const param::MIRParametrisation&);

    // -- Destructor

    virtual ~Gridded2RotatedOctahedralGG();

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
    // None

    // -- Overridden methods

    // From Gridded2GriddedInterpolation
    virtual void print(std::ostream&) const;

    // From Gridded2RotatedGrid
    virtual bool sameAs(const Action&) const;

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Members

    size_t N_;

    // -- Methods
    // None

    // -- Overridden methods

    // From Action
    virtual const char* name() const;

    // From Gridded2GriddedInterpolation
    virtual const repres::Representation* outputRepresentation() const;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace interpolate
}  // namespace action
}  // namespace mir


#endif
