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


#include "mir/method/knn/pick/LongestElementDiagonalAndNClosest.h"

#include "eckit/types/FloatCompare.h"
#include "eckit/utils/MD5.h"

#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/util/Exceptions.h"


namespace mir {
namespace method {
namespace knn {
namespace pick {


LongestElementDiagonalAndNClosest::LongestElementDiagonalAndNClosest(const param::MIRParametrisation& param) :
    nClosest_(4), distance_(0.), distance2_(0.), nClosestFirst_(false) {
    param.get("nclosest", nClosest_);
    ASSERT(nClosest_ > 0);
}


void LongestElementDiagonalAndNClosest::pick(const search::PointSearch& tree, const Point3& p,
                                             Pick::neighbours_t& closest) const {
    // This method switches between k- and distance-based searches for performance
    ASSERT(0. < distance_);

    if (nClosestFirst_) {
        tree.closestNPoints(p, nClosest_, closest);

        auto r2 = Point3::distance2(p, closest.back().point());
        if (r2 > distance2_) {
            tree.closestWithinRadius(p, distance_, closest);
            ASSERT(closest.size() <= nClosest_);

            nClosestFirst_ = false;
        }
    }
    else {
        tree.closestWithinRadius(p, distance_, closest);

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
    auto o = dynamic_cast<decltype(this)>(&other);
    return (o != nullptr) && nClosest_ == o->nClosest_ && eckit::types::is_approximately_equal(distance_, o->distance_);
}


void LongestElementDiagonalAndNClosest::distance(const repres::Representation& in) const {
    distance_ = 0.;
    ASSERT(in.getLongestElementDiagonal(distance_));
    ASSERT(0. < distance_);

    distance2_ = distance_ * distance_;
}


void LongestElementDiagonalAndNClosest::print(std::ostream& out) const {
    out << "LongestElementDistanceAndNClosest[nclosest=" << nClosest_ << ",distance=" << distance_ << "]";
}


void LongestElementDiagonalAndNClosest::hash(eckit::MD5& h) const {
    h.add("longest-element-diagonal-and-nclosest");
    h << nClosest_;
    h << distance_;
}


static PickBuilder<LongestElementDiagonalAndNClosest> __pick("longest-element-diagonal-and-nclosest");


}  // namespace pick
}  // namespace knn
}  // namespace method
}  // namespace mir
