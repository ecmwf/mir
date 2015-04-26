/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date Apr 2015

#include "mir/util/Increments.h"

#include <iostream>

#include "eckit/exception/Exceptions.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/util/Grib.h"

namespace mir {
namespace util {


Increments::Increments(double west_east, double north_south):
    west_east_(west_east),
    north_south_(north_south) {
}

Increments::Increments(const param::MIRParametrisation &parametrisation) {
    ASSERT(parametrisation.get("west_east_increment", west_east_));
    ASSERT(parametrisation.get("north_south_increment", north_south_));
}

Increments::~Increments() {
}

void Increments::print(std::ostream &out) const {
    out << "Increments["
        << "west_east=" << west_east_
        << ",north_south=" << north_south_
        << "]";
}

void Increments::fill(grib_info &info) const  {
    // Warning: scanning mode not considered
    info.grid.iDirectionIncrementInDegrees = west_east_;
    info.grid.jDirectionIncrementInDegrees = north_south_;
}


}  // namespace data
}  // namespace mir

