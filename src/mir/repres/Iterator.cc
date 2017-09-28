/*
 * (C) Copyright 1996-2017 ECMWF.
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
#include <vector>
#include "eckit/exception/Exceptions.h"
#include "atlas/util/Earth.h"


namespace mir {
namespace repres {


void mir::repres::Iterator::point_ll_t::print(std::ostream& s) const {
    s << "point_ll_t["
          "lat=" << lat
      << ",lon=" << lon
      << "]";
}


Iterator::Iterator(const util::Rotation& rotation) :
    valid_(true),
    rotation_(atlas::PointLonLat(
                  rotation.south_pole_longitude().normalise(Longitude::GREENWICH).value(),
                  rotation.south_pole_latitude().value() )) {
}


Iterator::~Iterator() {
}


const Iterator::point_2d_t& Iterator::operator*() const {
    ASSERT(valid_);
    return point_;
}


const Iterator::point_ll_t& Iterator::pointUnrotated() const {
    ASSERT(valid_);
    return pointUnrotated_;
}


Iterator& Iterator::next() {
    ASSERT(valid_);
    valid_ = next(pointUnrotated_.lat, pointUnrotated_.lon);

    if (valid_) {

        atlas::PointLonLat p(pointUnrotated_.lon.value(), pointUnrotated_.lat.value());
        rotation_.rotate(p.data());

        // notice the order
        point_[0] = p.lat();
        point_[1] = p.lon();
    }

    return *this;
}


const Iterator::point_3d_t Iterator::point3D() const {
    ASSERT(valid_);

    // notice the order
    const atlas::PointLonLat pll(point_[1], point_[0]);
    atlas::PointXYZ pxyz = atlas::util::Earth::convertGeodeticToGeocentric(pll);

    return pxyz;
}


void Iterator::print(std::ostream& out) const {
    out << "Iterator["
            "valid?" << valid_
        << ",pointUnrotated=" << pointUnrotated_
        << ",point=" << point_
        << ",rotated?" << rotation_.rotated()
        << ",rotation=" << rotation_
        << "]";
}


}  // namespace repres
}  // namespace mir
