// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/repres/latlon/LatLon.h"


namespace mir::repres::latlon {


class RegularLL : public LatLon {
public:
    // -- Exceptions
    // None

    // -- Constructors

    RegularLL(const param::MIRParametrisation&);
    RegularLL(const util::Increments&, const util::BoundingBox& = {}, const PointLatLon& reference = {0, 0});

    // -- Destructor

    ~RegularLL() override;

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
    // None

    // -- Methods
    // None

    // -- Overridden methods

    Iterator* iterator() const override;
    void print(std::ostream&) const override;
    void json(eckit::JSON&) const override;

    atlas::Grid atlasGrid() const override;

    void fillGrib(grib_info&) const override;
    void fillSpec(CustomSpec&) const override;

    void makeName(std::ostream&) const override;
    bool sameAs(const Representation&) const override;

    std::string intersectionOnCrop() const override { return "input-intersects-output"; }
    const RegularLL* croppedRepresentation(const util::BoundingBox&) const override;
    util::BoundingBox extendBoundingBox(const util::BoundingBox&) const override;

    std::vector<util::GridBox> gridBoxes() const override;

    std::string factory() const override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::repres::latlon
