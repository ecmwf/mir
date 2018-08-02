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
#include <vector>
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
    using atlas::PointLonLat;

    Point2 min, max;


    // 1. determine if poles are included (in the unrotated frame)
    const atlas::util::Rotation R(PointLonLat{
                                      south_pole_longitude_.normalise(Longitude::GREENWICH).value(),
                                      south_pole_latitude_.value() },
                                  south_pole_rotation_angle_);

    PointLonLat NP{ R.unrotate({0., Latitude::NORTH_POLE.value()}) };
    PointLonLat SP{ R.unrotate({0., Latitude::SOUTH_POLE.value()}) };

    const bool includesNorthPole = bbox.contains(NP.lat(), NP.lon());
    const bool includesSouthPole = bbox.contains(SP.lat(), SP.lon());


    // 2. determine box from rotated corners and by rotating each edge
    if (!includesNorthPole || !includesSouthPole) {

        std::vector<PointLonLat> corners {
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

        // locate edge extrema not at the corners, refining iteratively
        for (size_t i = 0; i < corners.size(); ++i) {
            const PointLonLat A = corners[i];
            const PointLonLat B = corners[(i + 1) % corners.size()];

            // finite difference vector derivative (H is the perturbation vector)
            constexpr double h = 0.001;
            const PointLonLat H{ Point2::mul(Point2::normalize(Point2::sub(B, A)), h) };

            auto derivate = [&R, &H](PointLonLat P) -> PointLonLat {
                PointLonLat F = R.rotate(P);
                PointLonLat Fh = R.rotate(PointLonLat::add(P, H));
                return PointLonLat::div(PointLonLat::sub(Fh, F),
                                        PointLonLat::norm(H));
            };

            const PointLonLat derivativeAtA = derivate(A);
            const PointLonLat derivativeAtB = derivate(B);

            // latitude extrema
            if (!eckit::types::is_strictly_greater(derivativeAtA.lat() * derivativeAtB.lat(), 0.)) {
                PointLonLat C = A;
                PointLonLat D = B;
                double derivativeAtC = derivativeAtA.lat();
                double derivativeAtD = derivativeAtB.lat();

                for (size_t cnt = 0; cnt < 100; ++cnt) {
                    PointLonLat M = PointLonLat::middle(C, D);
                    double derivativeAtM = derivate(M).lat();
                    if (eckit::types::is_strictly_greater(derivativeAtC * derivativeAtM, 0.)) {
                        C = M;
                        derivativeAtC = derivativeAtM;
                    } else if (eckit::types::is_strictly_greater(derivativeAtD * derivativeAtM, 0.)) {
                        D = M;
                        derivativeAtD = derivativeAtM;
                    } else {
                        break;
                    }
                }

                PointLonLat r(R.rotate(PointLonLat::middle(C, D)));
                min = Point2::componentsMin(min, r);
                max = Point2::componentsMax(max, r);
            }

            // longitude extrema
            if (!eckit::types::is_strictly_greater(derivativeAtA.lon() * derivativeAtB.lon(), 0.)) {
                PointLonLat C = A;
                PointLonLat D = B;
                double derivativeAtC = derivativeAtA.lon();
                double derivativeAtD = derivativeAtB.lon();

                for (size_t cnt = 0; cnt < 100; ++cnt) {
                    PointLonLat M = PointLonLat::middle(C, D);
                    double derivativeAtM = derivate(M).lon();
                    if (eckit::types::is_strictly_greater(derivativeAtC * derivativeAtM, 0.)) {
                        C = M;
                        derivativeAtC = derivativeAtM;
                    } else if (eckit::types::is_strictly_greater(derivativeAtD * derivativeAtM, 0.)) {
                        D = M;
                        derivativeAtD = derivativeAtM;
                    } else {
                        break;
                    }
                }

                PointLonLat r(R.rotate(PointLonLat::middle(C, D)));
                min = Point2::componentsMin(min, r);
                max = Point2::componentsMax(max, r);
            }
        }
    }


    // 4. extend by 'angle' latitude- and longitude-wise
    constexpr double angle = 0.001;  // (arbitrary choice)
    min = Point2::sub(min, Point2{ angle, angle });
    max = Point2::add(max, Point2{ angle, angle });


    // 5. set bounding box
    ASSERT(min[0] < max[0]);
    ASSERT(min[1] < max[1]);

    Latitude n = max[1];
    Latitude s = min[1];
    Longitude w = min[0];
    Longitude e = max[0];

    if (includesNorthPole || eckit::types::is_approximately_lesser_or_equal(Latitude::NORTH_POLE.value(), max[1])) {
        n = Latitude::NORTH_POLE;
    }

    if (includesSouthPole || eckit::types::is_approximately_greater_or_equal(Latitude::SOUTH_POLE.value(), min[1])) {
        s = Latitude::SOUTH_POLE;
    }

    if (includesNorthPole || includesSouthPole || eckit::types::is_approximately_lesser_or_equal(Longitude::GLOBE.value(), max[0] - min[0])) {
        w = 0;
        e = Longitude::GLOBE;
    }

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

