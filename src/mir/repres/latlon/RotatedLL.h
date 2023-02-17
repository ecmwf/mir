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
    void fillJob(api::MIRJob&) const override;

    void makeName(std::ostream&) const override;
    bool sameAs(const Representation&) const override;

    // From Representation
    const RotatedLL* croppedRepresentation(const util::BoundingBox&) const override;
    std::string factory() const override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::repres::latlon
