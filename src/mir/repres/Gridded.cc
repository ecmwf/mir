// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/repres/Gridded.h"

#include <string>

#include "eckit/geo/Grid.h"
#include "eckit/geo/order/Scan.h"
#include "eckit/geo/projection/Rotation.h"

#include "mir/util/Domain.h"
#include "mir/util/Exceptions.h"


namespace mir::repres {


Gridded::Gridded(const param::MIRParametrisation& parametrisation) : bbox_(parametrisation) {}


Gridded::Gridded(const util::BoundingBox& bbox) : bbox_(bbox) {}


Gridded::~Gridded() = default;


Gridded::Gridded() = default;


void Gridded::fillSpec(CustomSpec& spec) const {
    bbox_.fillSpec(spec);
}


namespace {


void reorder(MIRValuesVector& values, const eckit::geo::Grid& grid, const std::string& from, const std::string& to) {
    if (from == to) {
        return;
    }

    const auto ren = grid.reorder(from, to);
    ASSERT(values.size() == ren.size());

    MIRValuesVector out(values.size());
    for (size_t k = 0; k < ren.size(); ++k) {
        out[ren[k]] = values[k];
    }

    values.swap(out);
}


}  // namespace


void Gridded::reorderToCanonical(MIRValuesVector& values) const {
    if (valuesInDefaultOrder()) {
        reorder(values, grid(), grid().order(), eckit::geo::order::Scan::order_default());
    }
}


void Gridded::reorderFromCanonical(MIRValuesVector& values) const {
    if (valuesInDefaultOrder()) {
        reorder(values, grid(), eckit::geo::order::Scan::order_default(), grid().order());
    }
}


util::Domain Gridded::domain() const {
    const auto n = includesNorthPole() ? NORTH_POLE : bbox_.north();
    const auto s = includesSouthPole() ? SOUTH_POLE : bbox_.south();
    const auto w = bbox_.west();
    const auto e = isPeriodicWestEast() ? w + LONGITUDE_GLOBE : bbox_.east();

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
            .set("south_pole", std::vector<double>({rotation.south_pole().lon(), rotation.south_pole().lat()}))
            .set("rotation_angle", rotation.angle()));

    return {atlas::Grid::Spec(grid.spec()).set("projection", projection.spec())};
}


size_t Gridded::numberOfValues() const {
    return numberOfPoints();
}


}  // namespace mir::repres
