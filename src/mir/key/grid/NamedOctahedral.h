// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/key/grid/NamedGrid.h"


namespace mir::key::grid {


class NamedOctahedral : public NamedGrid {
public:
    // -- Exceptions
    // None

    // -- Constructors

    NamedOctahedral(const std::string& name, size_t N);

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
    // None

    // -- Overridden methods

    void print(std::ostream&) const override;
    size_t gaussianNumber() const override;
    const repres::Representation* representation() const override;
    const repres::Representation* representation(const util::Rotation&) const override;

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Members

    size_t N_;

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


}  // namespace mir::key::grid
