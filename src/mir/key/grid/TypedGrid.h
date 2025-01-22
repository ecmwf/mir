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

#include <set>

#include "mir/key/grid/Grid.h"


namespace mir::key::grid {


class TypedGrid : public Grid {
public:
    // -- Constructors

    TypedGrid(const std::string& key, const std::set<std::string>& requiredKeys,
              const std::set<std::string>& optionalKeys = {});

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

    std::set<std::string> requiredKeys_;
    std::set<std::string> optionalKeys_;

    // -- Methods

    void checkRequiredKeys(const param::MIRParametrisation&) const;

    // -- Overridden methods

    void print(std::ostream&) const override;
    const repres::Representation* representation(const param::MIRParametrisation&) const override = 0;
    void parametrisation(const std::string& grid, param::SimpleParametrisation&) const override;
    size_t gaussianNumber() const override;

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
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::key::grid
