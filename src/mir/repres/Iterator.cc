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

#include <vector>
#include "eckit/exception/Exceptions.h"


namespace mir {
namespace repres {


Iterator::Iterator() :
    valid_(true),
    rotation_(),
    projection_(atlas::Projection())
{
}


Iterator::Iterator(const util::Rotation& rotation) :
    valid_(true),
    rotation_(rotation) {
    // Setup projection using South Pole rotated position, as seen from the non-rotated frame
    const eckit::geometry::LLPoint2 pole(
        rotation_.south_pole_longitude().value(),
        rotation_.south_pole_latitude().value() );

    atlas::util::Config config;
    config.set("type", "rotated_lonlat");
    config.set("south_pole", std::vector<double>({pole.lon(), pole.lat()}));
    config.set("rotation_angle", rotation_.south_pole_rotation_angle());
    projection_ = atlas::Projection(config);
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
        if (projection_) {

            // notice the order
            const atlas::PointXY pxy(pointUnrotated_.lon.value(), pointUnrotated_.lat.value());
            const atlas::PointLonLat pll = projection_.lonlat(pxy);

            point_[0] = pll.lat();
            point_[1] = pll.lon();

        } else {
            point_[0] = pointUnrotated_.lat.value();
            point_[1] = pointUnrotated_.lon.value();
        }
        point_[0] = pointUnrotated_.lat.value();
        point_[1] = pointUnrotated_.lon.value();
    }

    return *this;
}


const Iterator::point_3d_t Iterator::point3D() const {
    ASSERT(valid_);

    // notice the order
    point_3d_t p;
    eckit::geometry::lonlat_to_3d(point_[1], point_[0], p.data());
    return p;
}


void Iterator::print(std::ostream& out) const {
    out << "Iterator["
        "valid?" << valid_
        << ",projection?" << bool(projection_)
        << ",rotation=" << rotation_
        << "]";
}


}  // namespace repres
}  // namespace mir
