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
