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

#include "mir/repres/regular/RegularGrid.h"


namespace mir::repres::regular {


class Lambert final : public RegularGrid {
public:
    // -- Constructors

    explicit Lambert(const param::MIRParametrisation&);

private:
    // -- Members

    double latitudeOfSouthernPoleInDegrees_;
    double longitudeOfSouthernPoleInDegrees_;
    bool writeLaDInDegrees_;
    bool writeLonPositive_;
    bool uvRelativeToGrid_;

    // -- Overridden methods

    void fillGrib(grib_info&) const override;
};


}  // namespace mir::repres::regular
