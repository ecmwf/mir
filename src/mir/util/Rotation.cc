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


namespace mir::util {


Rotation::Rotation(PointLonLat south_pole, double rotation_angle) : rotation_(south_pole, rotation_angle) {}


Rotation::Rotation(const param::MIRParametrisation& param) :
    Rotation(
        [](const auto& param) -> PointLonLat {
            PointLonLat sp{Longitude::GREENWICH.value(), Latitude::SOUTH_POLE.value()};
            if (std::vector<double> rotation; param.get("rotation", rotation)) {
                ASSERT_KEYWORD_ROTATION_SIZE(rotation.size());
                return {rotation[1], rotation[0]};
            }

            if (double lat = 0, lon = 0;
                param.get("south_pole_latitude", lat) && param.get("south_pole_longitude", lon)) {
                return {lon, lat};
            }

            return sp;
        }(param),
        [](const auto& param) -> double {
            double angle = 0.;
            param.get("south_pole_rotation_angle", angle);
            return angle;
        }(param)) {}


void Rotation::print(std::ostream& out) const {
    out << "Rotation["
        << "south_pole_latitude=" << rotation_.south_pole().lat
        << ",south_pole_longitude=" << rotation_.south_pole().lon << ",south_pole_rotation_angle=" << rotation_.angle()
        << "]";
}


void Rotation::fillGrib(grib_info& info) const {
    // Warning: scanning mode not considered

    info.grid.grid_type = CODES_UTIL_GRID_SPEC_ROTATED_LL;

    info.grid.latitudeOfSouthernPoleInDegrees  = rotation_.south_pole().lat;
    info.grid.longitudeOfSouthernPoleInDegrees = rotation_.south_pole().lon;

    // This is missing from the grib_spec
    // Remove that when supported
    if (!eckit::types::is_approximately_equal<double>(rotation_.angle(), 0.)) {
        info.extra_set("angleOfRotationInDegrees", rotation_.angle());
    }
}


void Rotation::fillJob(api::MIRJob& job) const {
    job.set("rotation", rotation_.south_pole().lat, rotation_.south_pole().lon);
}


bool Rotation::operator==(const Rotation& other) const {
    return points_equal(rotation_.south_pole(), other.rotation_.south_pole()) &&
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
    spec.set("south_pole", std::vector<double>({rotation_.south_pole().lon, rotation_.south_pole().lat}));
    spec.set("rotation_angle", rotation_.angle());

    return {grid.spec().set("projection", spec)};
#else
    NOTIMP;
#endif
}


BoundingBox Rotation::boundingBox(const BoundingBox& bbox) const {
    // use [0, 360[ longitude range if periodic
    std::unique_ptr<eckit::geo::area::BoundingBox> after(eckit::geo::area::BoundingBox::make_from_projection(
        PointLonLat{bbox.west().value(), bbox.south().value()}, PointLonLat{bbox.east().value(), bbox.north().value()},
        rotation_));

    return {after->north, after->periodic() ? Longitude::GREENWICH : after->west, after->south,
            after->periodic() ? Longitude::GLOBE : after->east};
}


void Rotation::makeName(std::ostream& out) const {
    out << "-rot:" << rotation_.south_pole().lat << ":" << rotation_.south_pole().lon << ":" << rotation_.angle();
}


}  // namespace mir::util
