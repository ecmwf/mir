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


#include "mir/util/Rotation.h"

#include <cmath>
#include <iostream>
#include <vector>
#include "eckit/exception/Exceptions.h"
#include "eckit/geometry/GreatCircle.h"
#include "eckit/types/FloatCompare.h"
#include "mir/api/Atlas.h"
#include "mir/api/MIRJob.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/BoundingBox.h"
#include "mir/util/Grib.h"


namespace mir {
namespace util {


Rotation::Rotation(const Latitude& south_pole_latitude,
                   const Longitude& south_pole_longitude,
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


Rotation::~Rotation() = default;


void Rotation::print(std::ostream& out) const {
    out << "Rotation["
        <<  "south_pole_latitude=" << south_pole_latitude_
        << ",south_pole_longitude=" << south_pole_longitude_
        << ",south_pole_rotation_angle=" << south_pole_rotation_angle_
        << "]";
}


void Rotation::fill(grib_info& info) const  {
    // Warning: scanning mode not considered

    info.grid.grid_type = GRIB_UTIL_GRID_SPEC_ROTATED_LL;

    info.grid.latitudeOfSouthernPoleInDegrees  = south_pole_latitude_.value();
    info.grid.longitudeOfSouthernPoleInDegrees = south_pole_longitude_.value();

    // This is missing from the grib_spec
    // Remove that when supported
    if (!eckit::types::is_approximately_equal<double>(south_pole_rotation_angle_, 0.)) {
        long j = info.packing.extra_settings_count++;
        info.packing.extra_settings[j].name = "angleOfRotationInDegrees";
        info.packing.extra_settings[j].type = GRIB_TYPE_DOUBLE;
        info.packing.extra_settings[j].double_value = south_pole_rotation_angle_;
    }
}


void Rotation::fill(api::MIRJob& job) const  {
    job.set("rotation", south_pole_latitude_.value(), south_pole_longitude_.value());
}


bool Rotation::operator==(const Rotation& other) const {
    return south_pole_latitude_ == other.south_pole_latitude_
           && south_pole_longitude_ == other.south_pole_longitude_
           && south_pole_rotation_angle_ == other.south_pole_rotation_angle_;
}


atlas::Grid Rotation::rotate(const atlas::Grid& grid) const {

    // ensure grid is not rotated already
    ASSERT(!grid.projection());

    atlas::util::Config config(grid.spec());
    config.set("projection", atlasProjection());

    return atlas::Grid(config);
}


atlas::Projection Rotation::atlasProjection() const {
    std::vector<double> p{south_pole_longitude_.value(), south_pole_latitude_.value()};

    atlas::util::Config config;
    config.set("type", "rotated_lonlat");
    config.set("south_pole", p);
    config.set("rotation_angle", south_pole_rotation_angle_);

    return atlas::Projection(config);
}


void Rotation::makeName(std::ostream& out) const {
    out << "-rot:"
        << south_pole_latitude_
        << ":"
        << south_pole_longitude_
        << ":"
        << south_pole_rotation_angle_;
}


}  // namespace data
}  // namespace mir

