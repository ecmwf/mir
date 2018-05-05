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


#include "mir/repres/Gridded.h"

#include "eckit/geometry/Point2.h"
#include "eckit/log/Log.h"
#include "mir/action/misc/AreaCropper.h"
#include "mir/api/Atlas.h"
#include "mir/config/LibMir.h"
#include "mir/util/Angles.h"
#include "mir/util/Domain.h"
#include "mir/util/Grib.h"
#include "mir/util/Rotation.h"


namespace mir {
namespace repres {


Gridded::Gridded() {}


Gridded::Gridded(const param::MIRParametrisation& parametrisation) :
    bbox_(parametrisation) {
}


Gridded::Gridded(const util::BoundingBox& bbox) :
    bbox_(bbox) {
}


Gridded::~Gridded() {}


void Gridded::setComplexPacking(grib_info& info) const {
    info.packing.packing_type = GRIB_UTIL_PACKING_TYPE_GRID_COMPLEX;
}


void Gridded::setSimplePacking(grib_info& info) const {
    info.packing.packing_type = GRIB_UTIL_PACKING_TYPE_GRID_SIMPLE;
}


void Gridded::setGivenPacking(grib_info&) const {
    // The packing_type is set by the caller
}


void Gridded::crop(const param::MIRParametrisation& parametrisation, context::Context& ctx) const {
    // only crop if not global
    if (!isGlobal()) {
        eckit::Log::info() << "+++++++++++ " << *this << " is not global" << std::endl;
        action::AreaCropper cropper(parametrisation, bbox_);
        cropper.execute(ctx);
    }
}


util::BoundingBox Gridded::extendedBoundingBox(const util::BoundingBox& bbox, double angle) const {

    // cropping bounding box after extending guarantees the representation can use it
    util::BoundingBox extended(bbox);
    Gridded::extendBoundingBox(extended, angle);

    return croppedBoundingBox(extended);
}


util::Domain Gridded::domain() const {

    const Latitude& n = includesNorthPole()? Latitude::NORTH_POLE : bbox_.north();
    const Latitude& s = includesSouthPole()? Latitude::SOUTH_POLE : bbox_.south();
    const Longitude& w = bbox_.west();
    const Longitude& e = isPeriodicWestEast()? bbox_.west() + Longitude::GLOBE : bbox_.east();

    return util::Domain(n, w, s, e);
}


const util::BoundingBox& Gridded::boundingBox() const {
    return bbox_;
}


util::BoundingBox Gridded::croppedBoundingBox(const util::BoundingBox& bbox) const {
    //FIMXE: does not respect the parameter 'caching'

    // set bounding box and inform
    bool caching = false;
    util::BoundingBox cropped = action::AreaCropper::croppingBoundingBox(this, bbox, caching);

    if (cropped != bbox) {
        eckit::Channel& log = eckit::Log::debug<LibMir>();
        std::streamsize old = log.precision(12);
        log << "Gridded::croppedBoundingBox: "
            << "\n   " << bbox
            << "\n > " << cropped
            << std::endl;
        log.precision(old);
    }

    return cropped;
}


bool Gridded::getLongestElementDiagonal(double&) const {
    return false;
}


void Gridded::extendBoundingBox(util::BoundingBox& bbox, double angle) {
    util::Rotation nonRotated;
    extendBoundingBox(bbox, angle, nonRotated);
}


void Gridded::extendBoundingBox(util::BoundingBox& bbox, double angle, const util::Rotation& rotation) {
    using namespace eckit::geometry;


    // Calculates a bbox in the un-rotated frame, containing the (possibly) rotated bbox;
    // First rotate the bbox corners, then expand to contain all elements covering the
    // original rotated box (for a valid interpolatation)


    // rotate bounding box corners and find (min, max)
    const atlas::PointLonLat southPole(
                rotation.south_pole_longitude().normalise(Longitude::GREENWICH).value(),
                rotation.south_pole_latitude().value() );

    const atlas::util::Rotation r(southPole);
    const atlas::PointLonLat p[] {
        r.rotate({bbox.west().value(), bbox.north().value()}),
        r.rotate({bbox.east().value(), bbox.north().value()}),
        r.rotate({bbox.east().value(), bbox.south().value()}),
        r.rotate({bbox.west().value(), bbox.south().value()})
    };

    Point2 min(p[0]);
    Point2 max(p[0]);
    for (size_t i = 1; i < 4; ++i) {
        min = Point2::componentsMin(min, p[i]);
        max = Point2::componentsMax(max, p[i]);
    }


    // for valid interpolations, extend by central 'angle' (converted from radius [m])
    ASSERT(angle >= 0);

    Latitude  n = max[1] + angle > Latitude::NORTH_POLE.value() ? Latitude::NORTH_POLE : max[1] + angle;
    Latitude  s = min[1] - angle < Latitude::SOUTH_POLE.value() ? Latitude::SOUTH_POLE : min[1] - angle;
    Longitude w = min[0];
    Longitude e = max[0];

    if ((Longitude::GLOBE + w - e).value() < 2. * angle) {
        e = Longitude::GLOBE + w;
    } else {
        w = min[0] - angle;
        e = max[0] + angle > (w + Longitude::GLOBE).value() ?
                    w + Longitude::GLOBE : Longitude(max[0] + angle);
    }


    // validate bounding box
    bbox = util::BoundingBox(n, w, s, e);
}


}  // namespace repres
}  // namespace mir

