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

#include "mir/method/MethodWeighted.h"


namespace mir {
namespace method {
namespace geo {


class GeographyMethod : public MethodWeighted {
public:
    // -- Types

    struct Biplet : std::pair<size_t, size_t> {
        using pair::pair;
        operator WeightMatrix::Triplet() const { return {first, second, 1. /*non-zero*/}; }
    };

    // -- Exceptions
    // None

    // -- Constructors

    using MethodWeighted::MethodWeighted;

    // -- Destructor
    // None

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    using Method::execute;

    // -- Overridden methods

    // From MethodWeighted
    bool sameAs(const Method&) const override;

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

    // From MethodWeighted
    void assemble(util::MIRStatistics&, WeightMatrix&, const repres::Representation& in,
                  const repres::Representation& out) const override = 0;
    const char* name() const override                               = 0;

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

    // From MethodWeighted
    void print(std::ostream&) const override;
    bool validateMatrixWeights() const override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace geo
}  // namespace method
}  // namespace mir
