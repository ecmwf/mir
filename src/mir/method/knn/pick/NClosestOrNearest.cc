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


#include "mir/method/knn/pick/NClosestOrNearest.h"

#include <cmath>

#include "eckit/log/JSON.h"
#include "eckit/types/FloatCompare.h"
#include "eckit/utils/MD5.h"

#include "mir/param/MIRParametrisation.h"
#include "mir/util/Exceptions.h"


namespace mir::method::knn::pick {


NClosestOrNearest::NClosestOrNearest(size_t nClosest, double distanceTolerance) :
    nClosest_(nClosest), distanceTolerance_(distanceTolerance) {
    ASSERT(nClosest_ > 0);
    ASSERT(distanceTolerance_ >= 0.);

    distanceTolerance2_ = distanceTolerance_ * distanceTolerance_;
}


NClosestOrNearest::NClosestOrNearest(const param::MIRParametrisation& param) {
    nClosest_ = 4;
    param.get("nclosest", nClosest_);
    ASSERT(nClosest_ > 0);

    distanceTolerance_ = 1.;
    param.get("distance-tolerance", distanceTolerance_);
    ASSERT(distanceTolerance_ >= 0.);

    distanceTolerance2_ = distanceTolerance_ * distanceTolerance_;
}


void NClosestOrNearest::pick(const search::PointSearch& tree, const Point3& p, Pick::neighbours_t& closest) const {
    auto n = nClosest_ == 1 ? 2 : nClosest_;
    tree.closestNPoints(p, n, closest);
    ASSERT(closest.size() == n);

    // if closest and farthest nb. are at the same distance, other points can
    // also be (like near poles), so we return all points inside radius
    auto nearest2  = Point3::distance2(p, closest.front().point());
    auto farthest2 = Point3::distance2(p, closest.back().point());
    if (eckit::types::is_approximately_equal(nearest2, farthest2, distanceTolerance2_)) {
        auto radius = std::sqrt(farthest2) + distanceTolerance_;
        tree.closestWithinRadius(p, radius, closest);
        return;
    }

    if (nClosest_ == 1) {
        // closest.resize(1);  // FIXME: better than below (but has black magic)
        closest.erase(closest.begin() + 1, closest.end());
    }
}


size_t NClosestOrNearest::n() const {
    return nClosest_;
}


bool NClosestOrNearest::sameAs(const Pick& other) const {
    const auto* o = dynamic_cast<const NClosestOrNearest*>(&other);
    return (o != nullptr) && nClosest_ == o->nClosest_ &&
           eckit::types::is_approximately_equal(distanceTolerance_, o->distanceTolerance_);
}


void NClosestOrNearest::json(eckit::JSON& j) const {
    j.startObject();
    j << "type" << "nclosest-or-nearest";
    j << "nclosest" << nClosest_;
    j << "distanceTolerance" << distanceTolerance_;
    j.endObject();
}


void NClosestOrNearest::print(std::ostream& out) const {
    out << "NClosestOrNearest[nclosest=" << nClosest_ << ",distanceTolerance=" << distanceTolerance_ << "]";
}


void NClosestOrNearest::hash(eckit::MD5& h) const {
    h.add(nClosest_);
    h.add(distanceTolerance_);
}


static const PickBuilder<NClosestOrNearest> __pick1("nclosest-or-nearest");
static const PickBuilder<NClosestOrNearest> __pick2("k");


}  // namespace mir::method::knn::pick
