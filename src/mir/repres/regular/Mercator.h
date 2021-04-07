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

#include "mir/repres/regular/RegularGrid.h"
#include "mir/util/Types.h"


namespace mir {
namespace repres {
namespace regular {


class Mercator : public RegularGrid {
public:
    // -- Exceptions
    // None

    // -- Constructors

    Mercator(const param::MIRParametrisation&);
    Mercator(const Mercator&) = delete;
    Mercator& operator=(const Mercator&) = delete;

    // -- Destructor
    // None

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

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

    static Projection make_projection(const param::MIRParametrisation&);

    // -- Overridden methods

    virtual void fill(grib_info&) const override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace regular
}  // namespace repres
}  // namespace mir
