// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <string>

#include "mir/repres/regular/RegularGrid.h"


namespace mir::repres::regular {


class PolarStereographic final : public RegularGrid {
public:
    // -- Exceptions
    // None

    // -- Constructors

    explicit PolarStereographic(const param::MIRParametrisation&);

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

    std::string proj_;
    double LaDInDegrees_;
    double orientationOfTheGridInDegrees_;
    bool southPoleOnProjectionPlane_;
    bool writeLaDInDegrees_;
    bool writeLonPositive_;
    bool uvRelativeToGrid_;

    // -- Methods
    // None

    // -- Overridden methods

    void fillGrib(grib_info&) const override;
    void fillSpec(CustomSpec&) const override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::repres::regular
