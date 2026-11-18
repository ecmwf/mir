// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/repres/Iterator.h"

#include <ostream>

#include "eckit/geo/projection/LonLatToXYZ.h"

#include "mir/util/Exceptions.h"


namespace mir {
namespace repres {


Iterator::Iterator(const util::Rotation& rotation) :
    rotation_({rotation.south_pole_longitude().value(), rotation.south_pole_latitude().value()},
              rotation.south_pole_rotation_angle()) {}


Iterator::~Iterator() = default;


const PointLonLat& Iterator::pointRotated() const {
    ASSERT(valid_);
    return rotated_;
}


const PointLatLon& Iterator::pointUnrotated() const {
    ASSERT(valid_);
    return unrotated_;
}


Iterator& Iterator::next() {
    ASSERT(valid_);
    PointLonLat p;
    valid_ = next(p);

    if (valid_) {
        unrotated_ = PointLatLon{p};
        rotated_   = rotation_.inv(unrotated_.lonlat());  // of the point as kept (see Longitude)
    }

    return *this;
}


PointXYZ Iterator::point_3D(const PointLonLat& point) {
    static const eckit::geo::projection::LonLatToXYZ to_xyz;
    return to_xyz.fwd(point);
}


PointXYZ Iterator::point3D() const {
    ASSERT(valid_);
    return point_3D(rotated_);
}


void Iterator::print(std::ostream& out) const {
    out << "Iterator["
           "valid?"
        << valid_ << ",PointLatLon=" << unrotated_ << ",point={" << rotated_.lat() << ',' << rotated_.lon()
        << "},rotated?" << rotation_.rotated() << ",rotation=" << rotation_.spec_str() << "]";
}


}  // namespace repres
}  // namespace mir
