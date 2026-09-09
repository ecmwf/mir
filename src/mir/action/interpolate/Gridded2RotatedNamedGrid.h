// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/action/interpolate/Gridded2RotatedGrid.h"


namespace mir::action::interpolate {


class Gridded2RotatedNamedGrid : public Gridded2RotatedGrid {
public:
    // -- Exceptions
    // None

    // -- Constructors

    explicit Gridded2RotatedNamedGrid(const param::MIRParametrisation&);

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
    // None

    // -- Overridden methods

    // From Gridded2GriddedInterpolation
    void print(std::ostream&) const override;

    // From Gridded2RotatedGrid
    bool sameAs(const Action&) const override;

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Members

    std::string grid_;

    // -- Methods
    // None

    // -- Overridden methods

    // From Action
    const char* name() const override;

    // From Gridded2GriddedInterpolation
    const repres::Representation* outputRepresentation() const override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::action::interpolate
