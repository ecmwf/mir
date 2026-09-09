// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/repres/gauss/regular/Regular.h"


namespace mir::repres::gauss::regular {


class RegularGG : public Regular {
public:
    // -- Exceptions
    // None

    // -- Constructors

    RegularGG(const param::MIRParametrisation&);
    RegularGG(size_t N, const util::BoundingBox& = util::BoundingBox(), double angularPrecision = 0);

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
    // None

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

    Iterator* iterator() const override;

    const Gridded* croppedRepresentation(const util::BoundingBox&) const override;

    bool sameAs(const Representation&) const override;

    std::string factory() const override;

    // From Representation
    std::vector<util::GridBox> gridBoxes() const override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::repres::gauss::regular
