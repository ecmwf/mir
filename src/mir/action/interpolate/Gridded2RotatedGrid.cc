/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/action/interpolate/Gridded2RotatedGrid.h"

#include <vector>
#include "eckit/exception/Exceptions.h"
#include "eckit/types/FloatCompare.h"
#include "mir/method/Method.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/api/Atlas.h"


namespace mir {
namespace action {
namespace interpolate {


Gridded2RotatedGrid::Gridded2RotatedGrid(const param::MIRParametrisation& parametrisation) :
    Gridded2GriddedInterpolation(parametrisation) {

    std::vector<double> value;
    ASSERT(parametrisation_.userParametrisation().get("rotation", value));

    ASSERT(value.size() == 2);
    rotation_ = util::Rotation(value[0], value[1]);
}


Gridded2RotatedGrid::~Gridded2RotatedGrid() = default;


const util::Rotation& Gridded2RotatedGrid::rotation() const {
    return rotation_;
}


const util::BoundingBox& Gridded2RotatedGrid::croppingBoundingBox() const {
    using eckit::geometry::Point2;

    repres::RepresentationHandle out(outputRepresentation());
    auto& bbox(method().hasCropping() ? method().getCropping()
                                      : out->boundingBox());


    // rotate bounding box corners and find (min, max)
    // include edges mid-points as they curve in the rotated frame
    const atlas::PointLonLat southPole(
                rotation_.south_pole_longitude().normalise(Longitude::GREENWICH).value(),
                rotation_.south_pole_latitude().value() );
    const atlas::util::Rotation R(southPole);

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

    Latitude n = includesNorthPole ? Latitude::NORTH_POLE : max[1];
    Latitude s = includesSouthPole ? Latitude::SOUTH_POLE : min[1];
    Longitude w = isPeriodicWestEast ? 0 : min[0];
    Longitude e = isPeriodicWestEast ? Longitude::GLOBE : max[0];

    bbox_ = util::BoundingBox(n, w, s, e);
    return bbox_;
}


bool Gridded2RotatedGrid::sameAs(const Action& other) const {
    auto o = dynamic_cast<const Gridded2RotatedGrid*>(&other);
    return o && (rotation_ == o->rotation_) && Gridded2GriddedInterpolation::sameAs(other);
}


}  // namespace interpolate
}  // namespace action
}  // namespace mir

