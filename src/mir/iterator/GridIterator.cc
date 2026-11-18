// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/iterator/GridIterator.h"

#include <cmath>
#include <ostream>
#include <variant>

#include "eckit/geo/Point.h"
#include "eckit/geo/order/Scan.h"
#include "eckit/geo/projection/Rotation.h"
#include "eckit/spec/Custom.h"
#include "eckit/spec/Layered.h"

#include "mir/util/Rotation.h"


namespace mir::iterator {


namespace {


const eckit::geo::projection::Rotation* rotation(const eckit::geo::Grid& grid) {
    const auto* r = dynamic_cast<const eckit::geo::projection::Rotation*>(&grid.projection());
    return r != nullptr && r->rotated() ? r : nullptr;
}


util::Rotation make_rotation(const eckit::geo::Grid& grid) {
    if (const auto* r = rotation(grid); r != nullptr) {
        const auto p = r->south_pole();
        return util::Rotation{p.lat(), p.lon(), r->angle()};
    }
    return util::Rotation{};
}


const eckit::geo::Grid* make_grid(const eckit::geo::Grid& grid, bool canonical) {
    // the same grid (by its spec, as not all grids support reordering), unrotated (grid frame) and in the default order
    const auto& order = eckit::geo::order::Scan::order_default();

    const bool rotated = rotation(grid) != nullptr;
    const bool reorder = canonical && grid.order() != order;
    if (!rotated && !reorder) {
        return nullptr;
    }

    eckit::spec::Layered spec(grid.spec());
    if (rotated) {
        spec.hide("projection");
        spec.hide("rotation");
    }

    if (reorder) {
        spec.push_front(new eckit::spec::Custom{{"order", order}});
    }

    return eckit::geo::GridFactory::build(spec);
}


}  // namespace


GridIterator::GridIterator(const eckit::geo::Grid& grid, bool canonical) :
    Iterator(make_rotation(grid)), grid_(make_grid(grid, canonical)), it_((grid_ ? *grid_ : grid).cbegin()) {}


void GridIterator::print(std::ostream& out) const {
    out << "GridIterator[";
    Iterator::print(out);
    out << ",index=" << it_.index() << "]";
}


bool GridIterator::next(PointLonLat& point) {
    if (!first_ && it_) {
        ++it_;
    }
    first_ = false;

    // points not on the Earth (eg. space view) are skipped
    for (; it_; ++it_) {
        if (const auto p = std::get<PointLonLat>(*it_); std::isfinite(p.lat()) && std::isfinite(p.lon())) {
            point = p;
            return true;
        }
    }

    return false;
}


}  // namespace mir::iterator
