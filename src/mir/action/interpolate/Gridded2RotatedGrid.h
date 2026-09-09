// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


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
