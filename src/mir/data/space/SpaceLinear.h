// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/data/Space.h"


namespace mir::data::space {


class SpaceLinear : public Space {
public:
    // -- Exceptions
    // None

    // -- Constructors

    SpaceLinear();

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
