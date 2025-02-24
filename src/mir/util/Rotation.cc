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


#include "mir/util/Rotation.h"

#include <ostream>
#include <vector>

#include "eckit/geo/area/BoundingBox.h"
#include "eckit/types/FloatCompare.h"

#include "mir/api/MIRJob.h"
#include "mir/api/mir_config.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/Atlas.h"
#include "mir/util/BoundingBox.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Grib.h"


namespace eckit::geo {
namespace area {
class BoundingBox;
}
class Projection;
namespace util {
area::BoundingBox bounding_box(Point2 min, Point2 max, const Projection&);
}
}  // namespace eckit::geo


namespace mir::util {


Rotation::Rotation(PointLonLat south_pole, double rotation_angle) : rotation_(south_pole, rotation_angle) {
    normalize();
}


Rotation::Rotation(const param::MIRParametrisation& parametrisation) {
    PointLonLat sp{Longitude::GREENWICH.value(), Latitude::SOUTH_POLE.value()};

    if (std::vector<double> rotation; parametrisation.userParametrisation().get("rotation", rotation)) {
        ASSERT_KEYWORD_ROTATION_SIZE(rotation.size());
        sp = {rotation[1], rotation[0]};
    }
    else if (double lat = 0, lon = 0;
             parametrisation.get("south_pole_latitude", lat) && parametrisation.get("south_pole_longitude", lon)) {
        sp = {lon, lat};
    }

    double angle = 0.;
    parametrisation.get("south_pole_rotation_angle", angle);

    rotation_ = eckit::geo::projection::Rotation(sp, angle);
    normalize();
}


void Rotation::normalize() {
    // south_pole_longitude_ = south_pole_longitude_.normalise(Longitude::GREENWICH);
}


void Rotation::print(std::ostream& out) const {
    out << "Rotation["
        << "south_pole_latitude=" << rotation_.southPole().lat << ",south_pole_longitude=" << rotation_.southPole().lon
        << ",south_pole_rotation_angle=" << rotation_.angle() << "]";
}


void Rotation::fillGrib(grib_info& info) const {
    // Warning: scanning mode not considered

    info.grid.grid_type = CODES_UTIL_GRID_SPEC_ROTATED_LL;

    info.grid.latitudeOfSouthernPoleInDegrees  = rotation_.southPole().lat;
    info.grid.longitudeOfSouthernPoleInDegrees = rotation_.southPole().lon;

    // This is missing from the grib_spec
    // Remove that when supported
    if (!eckit::types::is_approximately_equal<double>(rotation_.angle(), 0.)) {
        info.extra_set("angleOfRotationInDegrees", rotation_.angle());
    }
}


void Rotation::fillJob(api::MIRJob& job) const {
    job.set("rotation", rotation_.southPole().lat, rotation_.southPole().lon);
}


bool Rotation::operator==(const Rotation& other) const {
    return points_equal(rotation_.southPole(), other.rotation_.southPole()) &&
           rotation_.angle() == other.rotation_.angle();
}


PointLonLat Rotation::rotate(PointLonLat p) const {
    return rotation_.fwd(p);
}


atlas::Grid Rotation::rotate(const atlas::Grid& grid) const {
#if mir_HAVE_ATLAS
    // ensure grid is not rotated already
    ASSERT(!grid.projection());

    atlas::Projection::Spec spec;
    spec.set("type", "rotated_lonlat");
    spec.set("south_pole", std::vector<double>({rotation_.southPole().lon, rotation_.southPole().lat}));
    spec.set("rotation_angle", rotation_.angle());

    return {grid.spec().set("projection", spec)};
#else
    NOTIMP;
#endif
}


BoundingBox Rotation::boundingBox(const BoundingBox& bbox) const {
    // use [0, 360[ longitude range if periodic
    auto after = eckit::geo::util::bounding_box({bbox.west().value(), bbox.south().value()},
                                                {bbox.east().value(), bbox.north().value()}, rotation_);
    return {after.north, after.periodic() ? Longitude::GREENWICH : after.west, after.south,
            after.periodic() ? Longitude::GLOBE : after.east};
}


void Rotation::makeName(std::ostream& out) const {
    out << "-rot:" << rotation_.southPole().lat << ":" << rotation_.southPole().lon << ":" << rotation_.angle();
}


}  // namespace mir::util
