// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/repres/latlon/LatLon.h"
#include "mir/util/Rotation.h"


namespace mir::repres::latlon {


class RotatedLL : public LatLon {
public:
    // -- Exceptions
    // None

    // -- Constructors

    RotatedLL(const param::MIRParametrisation&);
    RotatedLL(const util::Increments&, const util::Rotation&, const util::BoundingBox& = {},
              const PointLatLon& reference = {0, 0});

    // -- Destructor

    ~RotatedLL() override;

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

    util::Rotation rotation_;

    // -- Methods
    // None

    // -- Overridden methods
    Iterator* iterator() const override;
    void print(std::ostream&) const override;

    atlas::Grid atlasGrid() const override;

    void fillGrib(grib_info&) const override;
    void fillSpec(CustomSpec&) const override;

    std::string intersectionOnCrop() const override { return "input-contains-output-check"; }
    const RotatedLL* croppedRepresentation(const util::BoundingBox&) const override;

    void makeName(std::ostream&) const override;
    bool sameAs(const Representation&) const override;

    std::string factory() const override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::repres::latlon
