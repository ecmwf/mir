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

    CalculateCellLongestDiagonal(std::string name = "cell_longest_diagonal", bool force_recompute = false);

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
