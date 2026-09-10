// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


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
    void fillSpec(CustomSpec&) const override;

    atlas::Grid atlasGrid() const override;
    Iterator* iterator() const override;

    std::string intersectionOnCrop() const override { return "input-contains-output-check"; }
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
