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
    void fillJob(api::MIRJob&) const override;

    void makeName(std::ostream&) const override;
    bool sameAs(const Representation&) const override;

    // From Representation
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
