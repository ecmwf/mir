/*
 * (C) Copyright 1996- ECMWF.
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


#include "mir/repres/Iterator.h"

#include <ostream>

#include "eckit/exception/Exceptions.h"

#include "mir/api/Atlas.h"


namespace mir {
namespace repres {


Iterator::Iterator(const util::Rotation& rotation) :
    rotation_(atlas::PointLonLat(
                  rotation.south_pole_longitude().normalise(Longitude::GREENWICH).value(),
                  rotation.south_pole_latitude().value() )),
    valid_(true) {
}


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

        atlas::PointLonLat p(lon_.value(), lat_.value());
        rotation_.rotate(p.data());

        // notice the order
        point_[0] = p.lat();
        point_[1] = p.lon();
    }

    return *this;
}


const Point3 Iterator::point3D() const {
    ASSERT(valid_);

    // notice the order
    const atlas::PointLonLat pll(point_[1], point_[0]);

    atlas::PointXYZ pxyz;
    atlas::util::Earth::convertSphericalToCartesian(pll, pxyz);

    return pxyz;
}


void Iterator::print(std::ostream& out) const {
    out << "Iterator["
            "valid?" << valid_
        << ",PointLatLon=";
    PointLatLon::print(out);
    out << ",point=" << point_
        << ",rotated?" << rotation_.rotated()
        << ",rotation=" << rotation_
        << "]";
}


}  // namespace repres
}  // namespace mir
