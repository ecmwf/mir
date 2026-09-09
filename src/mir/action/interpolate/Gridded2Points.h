// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <vector>

#include "mir/action/interpolate/Gridded2UnrotatedGrid.h"


namespace mir::action::interpolate {


class Gridded2Points : public Gridded2UnrotatedGrid {
public:
    // -- Exceptions
    // None

    // -- Constructors

    explicit Gridded2Points(const param::MIRParametrisation&);

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

private:
    // -- Members

    std::vector<double> latitudes_;
    std::vector<double> longitudes_;

    // -- Methods
    // None

    // -- Overridden methods

    bool sameAs(const Action&) const override;
    void print(std::ostream&) const override;

    const char* name() const override;

    const repres::Representation* outputRepresentation() const override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::action::interpolate
