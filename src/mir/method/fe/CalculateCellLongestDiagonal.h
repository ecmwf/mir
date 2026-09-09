// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <string>


namespace atlas {
class Mesh;
}  // namespace atlas


namespace mir::method::fe {


class CalculateCellLongestDiagonal {
public:
    // -- Types
    // None

    // -- Exceptions
    // None

    // -- Constructors

    explicit CalculateCellLongestDiagonal(std::string name = "cell_longest_diagonal", bool force_recompute = false);

    // -- Destructor
    // None

    // -- Convertors
    // None

    // -- Operators

    double operator()(atlas::Mesh&, bool include_virtual_points) const;

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
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Members

    std::string name_;
    bool force_recompute_;

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


}  // namespace mir::method::fe
