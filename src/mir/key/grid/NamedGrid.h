// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/key/grid/Grid.h"


namespace mir::key::grid {


class NamedGrid : public Grid {
protected:
    // -- Constructors

    explicit NamedGrid(const std::string& key) : Grid(key, "namedgrid") {}

    // -- Destructor
    // None

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods
    // None

    // -- Overridden methods

    void print(std::ostream&) const override                                           = 0;
    size_t gaussianNumber() const override                                             = 0;
    const repres::Representation* representation() const override                      = 0;
    const repres::Representation* representation(const util::Rotation&) const override = 0;

    std::string gridname() const override { return key(); }

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Friends
    // None
};


}  // namespace mir::key::grid
