/*
 * (C) Copyright 1996-2015 ECMWF.
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

#include <iostream>

#include "eckit/exception/Exceptions.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/util/Grib.h"
#include "mir/api/MIRJob.h"

namespace mir {
namespace util {


Rotation::Rotation(double south_pole_latitude,
                   double south_pole_longitude,
                   double south_pole_rotation_angle):
    south_pole_latitude_(south_pole_latitude),
    south_pole_longitude_(south_pole_longitude),
    south_pole_rotation_angle_(south_pole_rotation_angle) {
}

Rotation::Rotation(const param::MIRParametrisation &parametrisation) {
    ASSERT(parametrisation.get("south_pole_latitude", south_pole_latitude_));
    ASSERT(parametrisation.get("south_pole_longitude", south_pole_longitude_));
    ASSERT(parametrisation.get("south_pole_rotation_angle", south_pole_rotation_angle_));
}

Rotation::~Rotation() {
}

void Rotation::print(std::ostream &out) const {
    out << "Rotation["
        << "south_pole_latitude=" << south_pole_latitude_
        << ",south_pole_longitude=" << south_pole_longitude_
        << ",south_pole_rotation_angle=" << south_pole_rotation_angle_

        << "]";
}

void Rotation::fill(grib_info &info) const  {
    // Warning: scanning mode not considered

    info.grid.grid_type = GRIB_UTIL_GRID_SPEC_ROTATED_LL;

    // info.grid.uvRelativeToGrid= isec2[18]==8 ? 1 : 0;
    info.grid.latitudeOfSouthernPoleInDegrees    = south_pole_latitude_;
    info.grid.longitudeOfSouthernPoleInDegrees   = south_pole_longitude_;
    info.grid.uvRelativeToGrid = 1; // FIXME: only set for winds

    // This is missing from the grib_spec
    // Remove that when supported
    if (south_pole_rotation_angle_) {
        int n = info.packing.extra_settings_count++;
        info.packing.extra_settings[n].name = "angleOfRotationInDegrees";
        info.packing.extra_settings[n].type = GRIB_TYPE_DOUBLE;
        info.packing.extra_settings[n].long_value = south_pole_rotation_angle_;
    }
}

void Rotation::fill(api::MIRJob &job) const  {
    job.set("rotation", south_pole_latitude_, south_pole_longitude_);
}

bool Rotation::operator==(const Rotation &other) const {
    return south_pole_latitude_ == other.south_pole_latitude_
           && south_pole_longitude_ == other.south_pole_longitude_
           && south_pole_rotation_angle_ == other.south_pole_rotation_angle_;
}

}  // namespace data
}  // namespace mir

