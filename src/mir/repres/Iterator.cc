// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/repres/Iterator.h"

#include <ostream>

#include "eckit/geo/projection/LonLatToXYZ.h"

#include "mir/util/Exceptions.h"


namespace mir {
namespace repres {


Iterator::Iterator(const util::Rotation& rotation) :
    rotation_({rotation.south_pole_longitude().value(), rotation.south_pole_latitude().value()}), valid_(true) {}


Iterator::~Iterator() = default;


const Point2& Iterator::pointRotated() const {
    ASSERT(valid_);
    return point_;
}


const PointLatLon& Iterator::pointUnrotated() const {
    ASSERT(valid_);
    return *this;
}


Iterator& Iterator::next() {
    ASSERT(valid_);
    valid_ = next(lat_, lon_);

    if (valid_) {
        auto p = rotation_.fwd(eckit::geo::PointLonLat{lon_.value(), lat_.value()});

        // notice the order
        point_[0] = p.lat();
        point_[1] = p.lon();
    }

    return *this;
}


Point3 Iterator::point_3D(const Point2& point) {
    static const eckit::geo::projection::LonLatToXYZ to_xyz;

    // notice the order
    auto p = to_xyz.fwd(eckit::geo::PointLonLat{point[1], point[0]});

    return {p.X(), p.Y(), p.Z()};
}


Point3 Iterator::point3D() const {
    ASSERT(valid_);
    return point_3D(point_);
}


void Iterator::print(std::ostream& out) const {
    out << "Iterator["
           "valid?"
        << valid_ << ",PointLatLon=";
    PointLatLon::print(out);
    out << ",point=" << point_ << ",rotated?" << rotation_.rotated() << ",rotation=" << rotation_.spec_str() << "]";
}


}  // namespace repres
}  // namespace mir
