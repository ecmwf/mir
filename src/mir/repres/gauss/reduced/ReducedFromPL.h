// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/repres/gauss/reduced/FromPL.h"


namespace mir::repres::gauss::reduced {


class ReducedFromPL : public FromPL {
public:
    // -- Exceptions
    // None

    // -- Constructors

    ReducedFromPL(const param::MIRParametrisation&);
    ReducedFromPL(size_t, const std::vector<long>&, const util::BoundingBox& = util::BoundingBox(),
                  double angularPrecision = 0);

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

    void makeName(std::ostream&) const override;
    bool sameAs(const Representation&) const override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::repres::gauss::reduced
