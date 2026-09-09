// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/repres/regular/RegularGrid.h"


namespace mir::repres::regular {


class Lambert final : public RegularGrid {
public:
    // -- Exceptions
    // None

    // -- Constructors

    explicit Lambert(const param::MIRParametrisation&);

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

    double latitudeOfSouthernPoleInDegrees_;
    double longitudeOfSouthernPoleInDegrees_;
    bool writeLaDInDegrees_;
    bool writeLonPositive_;
    bool uvRelativeToGrid_;

    // -- Methods

    static Projection make_projection(const param::MIRParametrisation&);

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
