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

#include "eckit/geo/projection/LonLatToXYZ.h"

#include "mir/util/Exceptions.h"


namespace mir::repres {


static const eckit::geo::projection::LonLatToXYZ TO_XYZ;


Iterator::Iterator(const util::Rotation& rotation) :
    rotation_({rotation.south_pole_longitude().value(), rotation.south_pole_latitude().value()}), valid_(true) {}


const PointLonLat& Iterator::pointRotated() const {
    ASSERT(valid_);
    return pointRotated_;
}


const PointLonLat& Iterator::pointUnrotated() const {
    ASSERT(valid_);
    return pointUnrotated_;
}


Iterator& Iterator::next() {
    ASSERT(valid_);

    PointLonLat::value_type lon;
    PointLonLat::value_type lat;
    valid_ = next(lat, lon);

    if (valid_) {
        pointRotated_ = rotation_.fwd(pointUnrotated_ = {lon, lat});
    }

    return *this;
}


PointXYZ Iterator::point_3d(const PointLonLat& point) {
    auto p = TO_XYZ.fwd(point);

    return {p.X, p.Y, p.Z};
}


PointLonLat Iterator::point_ll(const PointXYZ& point) {
    return TO_XYZ.inv(eckit::geo::PointXYZ{point.x(0), point.x(1), point.x(2)});
}


PointXYZ Iterator::point3D() const {
    ASSERT(valid_);
    return point_3d(pointRotated_);
}


void Iterator::print(std::ostream& out) const {
    out << "Iterator["
           "valid?"
        << valid_ << ",PointLonLat=" << pointUnrotated_ << ",point=" << pointRotated_ << ",rotated?"
        << rotation_.rotated() << ",rotation=" << rotation_.spec_str() << "]";
}


}  // namespace mir::repres
