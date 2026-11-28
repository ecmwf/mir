// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/method/knn/pick/LongestElementDiagonalAndNClosest.h"

#include "eckit/log/JSON.h"
#include "eckit/types/FloatCompare.h"
#include "eckit/utils/MD5.h"

#include "mir/param/DefaultParametrisation.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/util/Exceptions.h"


namespace mir::method::knn::pick {


LongestElementDiagonalAndNClosest::LongestElementDiagonalAndNClosest(const param::MIRParametrisation& param) :
    nClosest_(param::DefaultParametrisation::instance().get_value<size_t>("nclosest", param)),
    distance_(0.),
    distance2_(0.),
    nClosestFirst_(false) {
    ASSERT(nClosest_ > 0);
}


void LongestElementDiagonalAndNClosest::pick(const eckit::geo::Search& tree, const PointXYZ& p,
                                             Pick::neighbours_t& closest) const {
    // This method switches between k- and distance-based searches for performance
    ASSERT(0. < distance_);

    if (nClosestFirst_) {
        closest = tree.search_knn(p, nClosest_);

        auto r2 = PointXYZ::distance2(p, closest.back().point);
        if (r2 > distance2_) {
            closest = tree.search_radius(p, distance_);
            ASSERT(closest.size() <= nClosest_);

            nClosestFirst_ = false;
        }
    }
    else {
        closest = tree.search_radius(p, distance_);

        if (closest.size() > nClosest_) {
            closest.erase(closest.begin() + long(nClosest_), closest.end());

            nClosestFirst_ = true;
        }
    }
}


size_t LongestElementDiagonalAndNClosest::n() const {
    return nClosest_;
}


bool LongestElementDiagonalAndNClosest::sameAs(const Pick& other) const {
    const auto* o = dynamic_cast<decltype(this)>(&other);
    return (o != nullptr) && nClosest_ == o->nClosest_ && eckit::types::is_approximately_equal(distance_, o->distance_);
}


void LongestElementDiagonalAndNClosest::distance(const repres::Representation& in) const {
    distance_ = 0.;
    ASSERT(in.getLongestElementDiagonal(distance_));
    ASSERT(0. < distance_);

    distance2_ = distance_ * distance_;
}


void LongestElementDiagonalAndNClosest::json(eckit::JSON& j) const {
    j << type() << "longest-element-diagonal-and-nclosest";
    param::DefaultParametrisation::instance().json(j, "nclosest", nClosest_);
}


void LongestElementDiagonalAndNClosest::print(std::ostream& out) const {
    out << "LongestElementDistanceAndNClosest[nclosest=" << nClosest_ << "]";
}


void LongestElementDiagonalAndNClosest::hash(eckit::MD5& h) const {
    h.add("longest-element-diagonal-and-nclosest");
    h << nClosest_;
}


static const PickBuilder<LongestElementDiagonalAndNClosest> __pick("longest-element-diagonal-and-nclosest");


}  // namespace mir::method::knn::pick
