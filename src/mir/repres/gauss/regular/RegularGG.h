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

    const Gridded* croppedRepresentation(const util::BoundingBox&) const override;
    bool sameAs(const Representation&) const override;
    Iterator* iterator() const override;
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
