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

#include "eckit/types/FloatCompare.h"

#include "mir/api/MIRJob.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/Atlas.h"
#include "mir/util/BoundingBox.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Grib.h"


namespace mir::util {


Rotation::Rotation(const Latitude& south_pole_latitude, const Longitude& south_pole_longitude,
                   double south_pole_rotation_angle) :
    south_pole_latitude_(south_pole_latitude),
    south_pole_longitude_(south_pole_longitude),
    south_pole_rotation_angle_(south_pole_rotation_angle) {

    normalize();
}


Rotation::Rotation(const param::MIRParametrisation& parametrisation) {

    double south_pole_latitude;
    ASSERT(parametrisation.get("south_pole_latitude", south_pole_latitude));
    south_pole_latitude_ = south_pole_latitude;

    double south_pole_longitude;
    ASSERT(parametrisation.get("south_pole_longitude", south_pole_longitude));
    south_pole_longitude_ = south_pole_longitude;

    south_pole_rotation_angle_ = 0.;
    ASSERT(parametrisation.get("south_pole_rotation_angle", south_pole_rotation_angle_));

    normalize();
}


void Rotation::normalize() {
    // south_pole_longitude_ = south_pole_longitude_.normalise(Longitude::GREENWICH);
}


void Rotation::print(std::ostream& out) const {
    out << "Rotation[" << "south_pole_latitude=" << south_pole_latitude_
        << ",south_pole_longitude=" << south_pole_longitude_
        << ",south_pole_rotation_angle=" << south_pole_rotation_angle_ << "]";
}


void Rotation::fillGrib(grib_info& info) const {
    // Warning: scanning mode not considered

    info.grid.grid_type = CODES_UTIL_GRID_SPEC_ROTATED_LL;

    info.grid.latitudeOfSouthernPoleInDegrees  = south_pole_latitude_.value();
    info.grid.longitudeOfSouthernPoleInDegrees = south_pole_longitude_.value();

    // This is missing from the grib_spec
    // Remove that when supported
    if (!eckit::types::is_approximately_equal<double>(south_pole_rotation_angle_, 0.)) {
        info.extra_set("angleOfRotationInDegrees", south_pole_rotation_angle_);
    }
}


void Rotation::fillJob(api::MIRJob& job) const {
    job.set("rotation", south_pole_latitude_.value(), south_pole_longitude_.value());
}


bool Rotation::operator==(const Rotation& other) const {
    return south_pole_latitude_ == other.south_pole_latitude_ && south_pole_longitude_ == other.south_pole_longitude_ &&
           south_pole_rotation_angle_ == other.south_pole_rotation_angle_;
}


atlas::Grid Rotation::rotate(const atlas::Grid& grid) const {

    // ensure grid is not rotated already
    ASSERT(!grid.projection());

    atlas::Grid::Spec spec(grid.spec());
    spec.set("projection", atlasProjection().spec());

    return {spec};
}


atlas::Projection Rotation::atlasProjection() const {
    atlas::Projection::Spec spec;

    spec.set("type", "rotated_lonlat");
    spec.set("south_pole", std::vector<double>({south_pole_longitude_.value(), south_pole_latitude_.value()}));
    spec.set("rotation_angle", south_pole_rotation_angle_);

    return {spec};
}


BoundingBox Rotation::boundingBox(const BoundingBox& bbox) const {

    atlas::RectangularDomain before({bbox.west().value(), bbox.east().value()},
                                    {bbox.south().value(), bbox.north().value()});
    ASSERT(before);

    auto after = atlasProjection().lonlatBoundingBox(before);
    ASSERT(after);

    // use [0, 360[ longitude range if periodic
    bool periodic = after.zonal_band();
    BoundingBox box(after.north(), periodic ? Longitude::GREENWICH : after.west(), after.south(),
                    periodic ? Longitude::GLOBE : after.east());

    return box;
}


void Rotation::makeName(std::ostream& out) const {
    out << "-rot:" << south_pole_latitude_ << ":" << south_pole_longitude_ << ":" << south_pole_rotation_angle_;
}


}  // namespace mir::util
