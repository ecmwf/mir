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

#include "mir/key/grid/GridPattern.h"


namespace mir::key::grid {


class HEALPixPattern : public GridPattern {
public:
    // -- Exceptions
    // None

    // -- Constructors

    HEALPixPattern(const std::string& name);
    HEALPixPattern(const HEALPixPattern&) = delete;

    // -- Destructor

    ~HEALPixPattern() override;

    // -- Convertors
    // None

    // -- Operators

    HEALPixPattern& operator=(const HEALPixPattern&) = delete;

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
    // None

    // -- Methods
    // None

    // -- Overridden methods

    void print(std::ostream&) const override;
    const Grid* make(const std::string&) const override;
    std::string canonical(const std::string&, const param::MIRParametrisation&) const override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::key::grid
