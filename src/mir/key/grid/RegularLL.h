// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/key/grid/Grid.h"

namespace mir::util {
class Increments;
}  // namespace mir::util


namespace mir::key::grid {


class RegularLL final : public Grid {
public:
    // -- Types
    // None

    // -- Exceptions
    // None

    // -- Constructors

    RegularLL(const std::string& key);

    // -- Destructor
    // None

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods
    // None

    // -- Overridden methods

    size_t gaussianNumber() const override;
    const repres::Representation* representation() const override;

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Members
    // None

    // -- Methods

    util::Increments increments() const;

    // -- Overridden methods

    void print(std::ostream&) const override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::key::grid
