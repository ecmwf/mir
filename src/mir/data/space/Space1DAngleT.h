// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


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
