/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#ifndef mir_action_interpolate_Gridded2RotatedGrid_h
#define mir_action_interpolate_Gridded2RotatedGrid_h

#include "mir/action/interpolate/Gridded2GriddedInterpolation.h"

#include "mir/util/BoundingBox.h"
#include "mir/util/Rotation.h"


namespace mir {
namespace action {
namespace interpolate {


class Gridded2RotatedGrid : public Gridded2GriddedInterpolation {
public:

    // -- Exceptions
    // None

    // -- Contructors

    Gridded2RotatedGrid(const param::MIRParametrisation&);

    // -- Destructor

    virtual ~Gridded2RotatedGrid();

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

    const util::Rotation& rotation() const;

    // -- Overridden methods

    // From Gridded2GriddedInterpolation
    virtual bool sameAs(const Action&) const;

    // -- Class members
    // None

    // -- Class methods
    // None

private:

    // -- Members

    util::Rotation rotation_;
    mutable util::BoundingBox bbox_;

    // -- Methods
    // None

    // -- Overridden methods

    // From Gridded2GriddedInterpolation
    virtual util::BoundingBox croppingBoundingBox() const;

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

