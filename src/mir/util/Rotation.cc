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

#include <iostream>
#include "eckit/exception/Exceptions.h"
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
    ASSERT(parametrisation.get("south_pole_latitude", south_pole_latitude_));
    ASSERT(parametrisation.get("south_pole_longitude", south_pole_longitude_));

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

    atlas::util::Config projection;
    projection.set("type", "rotated_lonlat");
    projection.set("south_pole", std::vector<double>({ south_pole_longitude_.value(), south_pole_latitude_.value() }));
    projection.set("rotation_angle", south_pole_rotation_angle_);

    atlas::util::Config config(grid.spec());
    config.set("projection", projection);

    return atlas::Grid(config);
}


BoundingBox Rotation::rotate(const BoundingBox& bbox) const {
    using eckit::geometry::Point2;

    // rotate bounding box corners and find (min, max)
    // include edges mid-points as they curve in the rotated frame
    const atlas::PointLonLat southPole(
                south_pole_longitude().normalise(Longitude::GREENWICH).value(),
                south_pole_latitude().value() );
    const atlas::util::Rotation R(southPole, south_pole_rotation_angle_);


    Point2 min, max;
    {
        const double n = bbox.north().value();
        const double s = bbox.south().value();
        const double w = bbox.west().value();
        const double e = bbox.east().value();
        const atlas::PointLonLat p[] {
            R.rotate({w, n}), R.rotate({(w + e) / 2., n}),
            R.rotate({e, n}), R.rotate({e, (s + n) / 2.}),
            R.rotate({e, s}), R.rotate({(w + e) / 2., s}),
            R.rotate({w, s}), R.rotate({w, (s + n) / 2.}),
        };

        min = max = p[0];
        for (size_t i = 1; i < 8; ++i) {
            min = Point2::componentsMin(min, p[i]);
            max = Point2::componentsMax(max, p[i]);
        }
    }
    ASSERT(min[0] < max[0]);
    ASSERT(min[1] < max[1]);


    //// extend by 'angle' latitude- and longitude-wise
    //constexpr double angle = 0.; //0.001 ??
    //min = Point2::add(min, Point2{ -angle, -angle });
    //max = Point2::add(max, Point2{  angle,  angle });


    // check bbox including poles (in the unrotated frame)
    atlas::PointLonLat NP{ R.unrotate({0., Latitude::NORTH_POLE.value()}) };
    atlas::PointLonLat SP{ R.unrotate({0., Latitude::SOUTH_POLE.value()}) };

    bool includesNorthPole = bbox.contains(NP.lat(), NP.lon())
            || eckit::types::is_approximately_lesser_or_equal(Latitude::NORTH_POLE.value(), max[1]);

    bool includesSouthPole = bbox.contains(SP.lat(), SP.lon())
            || eckit::types::is_approximately_greater_or_equal(Latitude::SOUTH_POLE.value(), min[1]);

    bool isPeriodicWestEast = includesNorthPole || includesSouthPole
            || eckit::types::is_approximately_lesser_or_equal(Longitude::GLOBE.value(), max[0] - min[0]);


    // set bounding box
    Latitude n = includesNorthPole ? Latitude::NORTH_POLE : max[1];
    Latitude s = includesSouthPole ? Latitude::SOUTH_POLE : min[1];
    Longitude w = isPeriodicWestEast ? 0 : min[0];
    Longitude e = isPeriodicWestEast ? Longitude::GLOBE : max[0];

    util::BoundingBox rotated(n, w, s, e);
    return rotated;
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

