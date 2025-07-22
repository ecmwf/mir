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

#include <memory>
#include <ostream>
#include <vector>

#include "eckit/geo/area/BoundingBox.h"
#include "eckit/types/FloatCompare.h"

#include "mir/api/MIRJob.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/Atlas.h"
#include "mir/util/BoundingBox.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Grib.h"


namespace mir::util {


namespace {


double get_double(const param::MIRParametrisation& param, const std::string& key) {
    double value{};
    ASSERT(param.get(key, value));
    return value;
}


}  // namespace


Rotation::Rotation(const Latitude& south_pole_latitude, const Longitude& south_pole_longitude,
                   double south_pole_rotation_angle) :
    rotation_({south_pole_longitude.value(), south_pole_latitude.value()}, south_pole_rotation_angle) {}


Rotation::Rotation(const param::MIRParametrisation& parametrisation) :
    rotation_({get_double(parametrisation, "south_pole_longitude"), get_double(parametrisation, "south_pole_latitude")},
              get_double(parametrisation, "south_pole_rotation_angle")) {}


void Rotation::print(std::ostream& out) const {
    out << "Rotation[spec=" << rotation_.spec_str() << "]";
}


void Rotation::fillGrib(grib_info& info) const {
    // Warning: scanning mode not considered

    info.grid.grid_type = CODES_UTIL_GRID_SPEC_ROTATED_LL;

    info.grid.latitudeOfSouthernPoleInDegrees  = rotation_.south_pole().lat;
    info.grid.longitudeOfSouthernPoleInDegrees = rotation_.south_pole().lon;
    info.grid.angleOfRotationInDegrees         = rotation_.angle();
}


void Rotation::fillJob(api::MIRJob& job) const {
    job.set("rotation", rotation_.south_pole().lat, rotation_.south_pole().lon);
}


bool Rotation::operator==(const Rotation& other) const {
    return rotation_ == other.rotation_;
}


BoundingBox Rotation::boundingBox(const BoundingBox& bbox) const {
    eckit::geo::projection::Rotation projection({south_pole_longitude().value(), south_pole_latitude().value()});

    auto after = eckit::geo::area::BoundingBox::make_from_projection(
        {bbox.west().value(), bbox.south().value()}, {bbox.east().value(), bbox.north().value()}, projection);
    ASSERT(after);

    return {after->north, after->west, after->south, after->east};
}


void Rotation::makeName(std::ostream& out) const {
    out << "-rot:" << rotation_.south_pole().lat << ":" << rotation_.south_pole().lon << ":" << rotation_.angle();
}


}  // namespace mir::util
