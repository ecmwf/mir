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

#include "mir/util/Shift.h"

#include <iostream>

#include "eckit/exception/Exceptions.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/util/Grib.h"
#include "mir/api/MIRJob.h"
#include "eckit/types/Fraction.h"
#include "mir/util/BoundingBox.h"

namespace mir {
namespace util {


Shift::Shift(const eckit::Fraction& west_east, const eckit::Fraction& south_north):
    west_east_(west_east),
    south_north_(south_north) {
    // ASSERT(west_east_ > 0);
    // ASSERT(south_north_ > 0);
}

Shift::~Shift() {
}

void Shift::print(std::ostream &out) const {
    out << "Shift["
        << "west_east=" << double(west_east_)
        << ",south_north=" << double(south_north_)
        << "]";
}


void Shift::fill(api::MIRJob &job) const  {
    job.set("shift", west_east_, south_north_);
}

void Shift::makeName(std::ostream& out) const {
    out << "-shift:"
        << double(west_east_)
        << ":"
        << double(south_north_)
        ;
}



}  // namespace data
}  // namespace mir

