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

    atlas::util::Config projection;
    projection.set("type", "rotated_lonlat");
    projection.set("south_pole", std::vector<double>({ south_pole_longitude_.value(), south_pole_latitude_.value() }));
    projection.set("rotation_angle", south_pole_rotation_angle_);

    atlas::util::Config config(grid.spec());
    config.set("projection", projection);

    return atlas::Grid(config);
}


BoundingBox Rotation::rotate(const BoundingBox& bbox) const {
    using atlas::PointLonLat;

    using eckit::types::is_approximately_lesser_or_equal;
    using eckit::types::is_strictly_greater;


    // 0. setup
    Point2 min, max;

    const atlas::util::Rotation R(PointLonLat{
                                      south_pole_longitude_.normalise(Longitude::GREENWICH).value(),
                                      south_pole_latitude_.value() },
                                  south_pole_rotation_angle_);

    constexpr double h = 0.001;

    auto derivate = [&R](PointLonLat P, const PointLonLat& H) -> PointLonLat {
        const bool backwards =
                P.lat() + H.lat() > Latitude::NORTH_POLE.value() ||
                P.lat() + H.lat() < Latitude::SOUTH_POLE.value();
        const PointLonLat F[2] = {
            backwards? R.rotate(PointLonLat::sub(P, H)) : R.rotate(P),
            backwards? R.rotate(P) : R.rotate(PointLonLat::add(P, H))
        };
        return PointLonLat::div(PointLonLat::sub(F[1], F[0]), PointLonLat::norm(H));
    };


    // 1. determine box from rotated corners
    const std::vector<PointLonLat> corners {
        {bbox.west().value(), bbox.north().value()},
        {bbox.east().value(), bbox.north().value()},
        {bbox.east().value(), bbox.south().value()},
        {bbox.west().value(), bbox.south().value()}
    };

    bool first = true;
    for (auto& p : corners) {
        PointLonLat r(R.rotate(p));
        min = first ? r : Point2::componentsMin(min, r);
        max = first ? r : Point2::componentsMax(max, r);
        first = false;
    }


    // 2. locate latitude extrema by checking if poles are included (in the
    // unrotated frame) and if not, find extrema not at the corners by refining
    // iteratively

    PointLonLat NP{ R.unrotate({0., Latitude::NORTH_POLE.value()}) };
    PointLonLat SP{ R.unrotate({0., Latitude::SOUTH_POLE.value()}) };

    bool includesNorthPole = bbox.contains(NP.lat(), NP.lon());
    bool includesSouthPole = bbox.contains(SP.lat(), SP.lon());

    if (!includesNorthPole || !includesSouthPole) {

        for (size_t i = 0; i < corners.size(); ++i) {
            PointLonLat A = corners[i];
            PointLonLat B = corners[(i + 1) % corners.size()];

            // finite difference vector derivative (H is the perturbation vector)
            const PointLonLat H{ Point2::mul(Point2::normalize(Point2::sub(B, A)), h) };
            double derivativeAtA = derivate(A, H).lat();
            double derivativeAtB = derivate(B, H).lat();

            if (!is_strictly_greater(derivativeAtA * derivativeAtB, 0.)) {
                for (size_t cnt = 0; cnt < 100; ++cnt) {
                    PointLonLat M = PointLonLat::middle(A, B);
                    double derivativeAtM = derivate(M, H).lat();
                    if (is_strictly_greater(derivativeAtA * derivativeAtM, 0.)) {
                        A = M;
                        derivativeAtA = derivativeAtM;
                    } else if (is_strictly_greater(derivativeAtB * derivativeAtM, 0.)) {
                        B = M;
                        derivativeAtB = derivativeAtM;
                    } else {
                        break;
                    }
                }

                PointLonLat r(R.rotate(PointLonLat::middle(A, B)));
                min = Point2::componentsMin(min, r);
                max = Point2::componentsMax(max, r);
            }
        }

        // extend by 'a small amount' (arbitrary)
        min = Point2::sub(min, Point2{ 0, h });
        max = Point2::add(max, Point2{ 0, h });

        includesNorthPole = includesNorthPole || is_approximately_lesser_or_equal(Latitude::NORTH_POLE.value(), max[1]);
        includesSouthPole = includesSouthPole || is_approximately_lesser_or_equal(min[1], Latitude::SOUTH_POLE.value());
    }
    ASSERT(min[1] < max[1]);


    // 3. locate latitude extrema by checking if date line is crossed (in the
    // unrotated frame), in which case we assume periodicity and if not, find
    // extrema not at the corners by refining iteratively

    bool crossesDateLine = includesNorthPole || includesSouthPole;

    if (!crossesDateLine) {
        PointLonLat A { R.unrotate({Longitude::DATE_LINE.value(), -10}) };
        PointLonLat B { R.unrotate({Longitude::DATE_LINE.value(), 10}) };
        eckit::geometry::GreatCircle DL(A, B);

        for (auto lon : { bbox.west(), bbox.east() }) {
            if (!crossesDateLine) {
                for (auto lat : DL.latitude(lon.value())) {
                    if ((crossesDateLine = bbox.contains(lat, lon))) {
                        break;
                    }
                }
            }
        }

        for (auto lat : { bbox.north(), bbox.south() }) {
            if (!crossesDateLine) {
                for (auto lon : DL.longitude(lat.value())) {
                    if ((crossesDateLine = bbox.contains(lat, lon))) {
                        break;
                    }
                }
            }
        }
    }

    if (!crossesDateLine) {

        for (size_t i = 0; i < corners.size(); ++i) {
            PointLonLat A = corners[i];
            PointLonLat B = corners[(i + 1) % corners.size()];

            // finite difference vector derivative (H is the perturbation vector)
            const PointLonLat H{ Point2::mul(Point2::normalize(Point2::sub(B, A)), h) };
            double derivativeAtA = derivate(A, H).lon();
            double derivativeAtB = derivate(B, H).lon();

            if (!is_strictly_greater(derivativeAtA * derivativeAtB, 0.)) {
                for (size_t cnt = 0; cnt < 100; ++cnt) {
                    PointLonLat M = PointLonLat::middle(A, B);
                    double derivativeAtM = derivate(M, H).lon();
                    if (is_strictly_greater(derivativeAtA * derivativeAtM, 0.)) {
                        A = M;
                        derivativeAtA = derivativeAtM;
                    } else if (is_strictly_greater(derivativeAtB * derivativeAtM, 0.)) {
                        B = M;
                        derivativeAtB = derivativeAtM;
                    } else {
                        break;
                    }
                }

                PointLonLat r(R.rotate(PointLonLat::middle(A, B)));
                min = Point2::componentsMin(min, r);
                max = Point2::componentsMax(max, r);
            }
        }

        // extend by 'a small amount' (arbitrary)
        min = Point2::sub(min, Point2{ h, 0 });
        max = Point2::add(max, Point2{ h, 0 });

        crossesDateLine = is_approximately_lesser_or_equal(Longitude::GLOBE.value(), max[0] - min[0]);
    }
    ASSERT(min[0] < max[0]);


    // 4. set bounding box
    Latitude n = includesNorthPole ? Latitude::NORTH_POLE : max[1];
    Latitude s = includesSouthPole ? Latitude::SOUTH_POLE : min[1];
    Longitude w = crossesDateLine ? 0 : min[0];
    Longitude e = crossesDateLine ? Longitude::GLOBE : max[0];

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

