// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/repres/gauss/reduced/Reduced.h"
#include "mir/util/BoundingBox.h"


namespace mir::repres::gauss::reduced {


class FromPL : public Reduced {
public:
    // -- Exceptions
    // None

    // -- Constructors

    FromPL(const param::MIRParametrisation&);
    FromPL(size_t, const std::vector<long>&, const util::BoundingBox& = util::BoundingBox(),
           double angularPrecision = 0);

    // -- Destructor

    ~FromPL() override = default;

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

protected:
    // -- Members
    // None

    // -- Methods
    // None

    // -- Overridden methods
    atlas::Grid atlasGrid() const override;
    void makeName(std::ostream&) const override;
    bool sameAs(const Representation&) const override;

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
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::repres::gauss::reduced
