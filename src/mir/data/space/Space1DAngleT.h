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

#include "mir/data/Space.h"

namespace mir::data::space {


template <int SCALE, int SYMMETRY>
class Space1DAngleT : public Space {
public:
    // -- Exceptions
    // None

    // -- Constructors

    Space1DAngleT();

    // -- Destructor
    // None

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods
    // None

    // -- Overridden methods

    void linearise(const Space::Matrix&, Space::Matrix&, double missingValue) const override;
    void unlinearise(const Space::Matrix&, Space::Matrix&, double missingValue) const override;
    size_t dimensions() const override;

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Types
    // None

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


}  // namespace mir::data::space
