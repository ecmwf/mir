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


#include <iostream>

#include "eckit/exception/Exceptions.h"

#include "soyuz/util/BoundingBox.h"
#include "soyuz/repres/Representation.h"


namespace mir {
namespace util {


BoundingBox::BoundingBox(double north,
                         double west,
                         double south,
                         double east):
    north_(north),
    west_(west),
    south_(south),
    east_(east) {
}



void BoundingBox::print(std::ostream &out) const {
    out << "BoundingBox["

        << "north=" << north_
        << ",west=" << west_
        << ",south=" << south_
        << ",east=" << east_
        << "]";
}




}  // namespace data
}  // namespace mir

