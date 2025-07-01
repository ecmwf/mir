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


#include "mir/repres/Iterator.h"

#include <ostream>

#include "mir/util/Exceptions.h"


namespace mir::repres {


Iterator::Iterator(const util::Rotation& rotation) :
    rotation_({rotation.south_pole_longitude(), rotation.south_pole_latitude()}), valid_(true) {}


Iterator::~Iterator() = default;


const PointLonLat& Iterator::pointRotated() const {
    ASSERT(valid_);
    return pointRotated_;
}


const PointLonLat& Iterator::pointUnrotated() const {
    ASSERT(valid_);
    return point_;
}


Iterator& Iterator::next() {
    ASSERT(valid_);

    if (point_ = next(valid_); valid_) {
        pointRotated_ = rotation_.fwd(point_);
    }

    return *this;
}


void Iterator::print(std::ostream& out) const {
    out << "Iterator["
           "valid?"
        << valid_ << ",point=" << point_ << ",pointRotated=" << pointRotated_ << ",rotated?" << rotation_.rotated()
        << ",rotation=" << rotation_.spec_str() << "]";
}


}  // namespace mir::repres
