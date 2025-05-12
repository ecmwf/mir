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


#include "mir/repres/Gridded.h"

#include "eckit/geo/projection/Rotation.h"

#include "mir/util/Domain.h"
#include "mir/util/Exceptions.h"


namespace mir::repres {


Gridded::Gridded(const param::MIRParametrisation& parametrisation) : bbox_(parametrisation) {}


Gridded::Gridded(const util::BoundingBox& bbox) : bbox_(bbox) {}


Gridded::~Gridded() = default;


Gridded::Gridded() = default;


util::Domain Gridded::domain() const {
    const Latitude& n  = includesNorthPole() ? Latitude::NORTH_POLE : bbox_.north();
    const Latitude& s  = includesSouthPole() ? Latitude::SOUTH_POLE : bbox_.south();
    const Longitude& w = bbox_.west();
    const Longitude& e = isPeriodicWestEast() ? bbox_.west() + Longitude::GLOBE : bbox_.east();

    return util::Domain(n, w, s, e);
}


const util::BoundingBox& Gridded::boundingBox() const {
    return bbox_;
}


bool Gridded::getLongestElementDiagonal(double& /*unused*/) const {
    return false;
}


bool Gridded::crop(util::BoundingBox& /*unused*/, util::IndexMapping& /*unused*/) const {
    return false;
}


atlas::Grid Gridded::rotate_atlas_grid(const Rotation& rotation, const atlas::Grid& grid) {
    // ensure grid is not rotated already
    ASSERT(!grid.projection());

    atlas::Projection projection(
        atlas::Projection::Spec{"type", "rotated_lonlat"}
            .set("south_pole", std::vector<double>({rotation.south_pole().lon, rotation.south_pole().lat}))
            .set("rotation_angle", rotation.angle()));

    return {atlas::Grid::Spec(grid.spec()).set("projection", projection.spec())};
}


size_t Gridded::numberOfValues() const {
    return numberOfPoints();
}


}  // namespace mir::repres
