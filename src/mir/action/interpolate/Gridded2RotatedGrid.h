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


#pragma once

#include "mir/action/interpolate/Gridded2GriddedInterpolation.h"
#include "mir/util/BoundingBox.h"
#include "mir/util/Rotation.h"


namespace mir::action::interpolate {


class Gridded2RotatedGrid : public Gridded2GriddedInterpolation {
public:
    // -- Exceptions
    // None

    // -- Constructors

    explicit Gridded2RotatedGrid(const param::MIRParametrisation&);

    // -- Destructor
    // None


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
    bool sameAs(const Action&) const override;

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
    util::BoundingBox outputBoundingBox() const override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::action::interpolate
