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


Increments::Increments(double west_east, double south_north):
    west_east_(west_east),
    south_north_(south_north) {
}

Increments::Increments(const param::MIRParametrisation &parametrisation) {
    ASSERT(parametrisation.get("west_east_increment", west_east_));
    ASSERT(parametrisation.get("south_north_increment", south_north_));
}

Increments::~Increments() {
}

void Increments::print(std::ostream &out) const {
    out << "Increments["
        << "west_east=" << west_east_
        << ",south_north=" << south_north_
        << "]";
}

void Increments::fill(grib_info &info) const  {
    // Warning: scanning mode not considered
    info.grid.iDirectionIncrementInDegrees = west_east_;
    info.grid.jDirectionIncrementInDegrees = south_north_;
}


}  // namespace data
}  // namespace mir

