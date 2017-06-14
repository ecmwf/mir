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


#include "mir/util/ShiftIterator.h"
#include "eckit/types/FloatCompare.h"


namespace mir {
namespace util {


ShiftIterator::ShiftIterator(Iterator* iterator, const util::Shift& shift) :
    iterator_(iterator),
    shift_(shift) {
}


ShiftIterator::~ShiftIterator() {
}


void ShiftIterator::print(std::ostream& out) const {
    out << "ShiftIterator[iterator=" << *iterator_
        << ",shift=" << shift_ << "]";
}


bool ShiftIterator::next(Latitude &lat, Longitude &lon) {


    const double eastwards = shift_.west_east();
    const double northwards = shift_.south_north();


    while (iterator_->next(lat, lon)) {

        lon += eastwards;
        lat += northwards;

        if (eckit::types::is_strictly_greater(lat, 90.) || eckit::types::is_strictly_greater(-90., lat)) {
            continue;
        }


        return true;
    }
    return false;
}


}  // namespace repres
}  // namespace mir

