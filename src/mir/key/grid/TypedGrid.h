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


#ifndef mir_key_grid_TypedGrid_h
#define mir_key_grid_TypedGrid_h

#include "mir/key/grid/Grid.h"

#include <vector>


namespace mir {
namespace key {
namespace grid {


class TypedGrid : public Grid {
public:
    // -- Constructors

    TypedGrid(const std::string& key, const std::vector<std::string>& requiredKeys);

    // -- Destructor

    virtual ~TypedGrid() = default;

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

    std::vector<std::string> requiredKeys_;

    // -- Methods
    // None

    // -- Overridden methods

    virtual void print(std::ostream&) const override;
    virtual const repres::Representation* representation(const param::MIRParametrisation&) const override = 0;
    virtual size_t gaussianNumber() const override;

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


}  // namespace grid
}  // namespace key
}  // namespace mir


#endif
