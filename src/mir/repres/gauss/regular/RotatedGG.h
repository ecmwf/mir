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

#include "mir/repres/gauss/regular/Regular.h"
#include "mir/util/Rotation.h"


namespace mir::repres::gauss::regular {


class RotatedGG : public Regular {
public:
    // -- Exceptions
    // None

    // -- Constructors

    RotatedGG(const param::MIRParametrisation&);
    RotatedGG(size_t N, const util::Rotation&, const util::BoundingBox& = util::BoundingBox(),
              double angularPrecision = 0);

    // -- Destructor
    // None

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

protected:
    // -- Members

    util::Rotation rotation_;

    // -- Methods

    void print(std::ostream&) const override;

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

    void fillGrib(grib_info&) const override;
    void fillJob(api::MIRJob&) const override;

    atlas::Grid atlasGrid() const override;
    Iterator* iterator() const override;

    const Gridded* croppedRepresentation(const util::BoundingBox&) const override;
    void makeName(std::ostream&) const override;
    bool sameAs(const Representation&) const override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::repres::gauss::regular
