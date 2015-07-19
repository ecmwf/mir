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


#include "mir/util/RotatedIterator.h"


namespace mir {
namespace util {


RotatedIterator::RotatedIterator(Iterator *iterator, const util::Rotation &rotation):
    iterator_(iterator),
    rotation_(rotation),
    rotate_(eckit::geometry::LLPoint2(rotation.south_pole_longitude(), rotation.south_pole_latitude()),
            rotation.south_pole_rotation_angle()) {
}


RotatedIterator::~RotatedIterator() {
}

void RotatedIterator::print(std::ostream &out) const {
    out << "RotatedIterator[iterator=" << *iterator_ << ",rotation=" << rotation_ << "]";
}

bool RotatedIterator::next(double &lat, double &lon) {

    if(iterator_->next(lat, lon)) {
        eckit::geometry::LLPoint2 p = rotate_.unrotate(eckit::geometry::LLPoint2(lon, lat)); // <== notice order
        lat = p.lat();
        lon = p.lon();
        return true;
    }

    return false;
}


}  // namespace repres
}  // namespace mir

