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

#include <iosfwd>

#include "mir/util/Types.h"


namespace mir {
namespace repres {
class Representation;
}
namespace util {
class Rotation;
}
}  // namespace mir


namespace mir::data {


class CartesianVector2DField {
public:
    // -- Exceptions
    // None

    // -- Constructors

    explicit CartesianVector2DField(const repres::Representation*, bool hasMissing = false, double missingValue = 0);

    CartesianVector2DField(const CartesianVector2DField&) = delete;
    CartesianVector2DField(CartesianVector2DField&&)      = delete;

    // -- Destructor

    ~CartesianVector2DField();

    // -- Convertors
    // None

    // -- Operators

    void operator=(const CartesianVector2DField&) = delete;
    void operator=(CartesianVector2DField&&)      = delete;

    // -- Methods

    void rotate(const util::Rotation&, MIRValuesVector& valuesX, MIRValuesVector& valuesY) const;

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

    void print(std::ostream&) const;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Members

    MIRValuesVector valuesX_;
    MIRValuesVector valuesY_;
    double missingValue_;
    const repres::Representation* representation_;

    // -- Methods
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    friend std::ostream& operator<<(std::ostream& s, const CartesianVector2DField& p) {
        p.print(s);
        return s;
    }
};


}  // namespace mir::data
