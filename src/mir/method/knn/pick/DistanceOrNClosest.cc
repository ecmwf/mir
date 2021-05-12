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


#include "mir/method/knn/pick/DistanceOrNClosest.h"

#include "eckit/types/FloatCompare.h"
#include "eckit/utils/MD5.h"

#include "mir/param/MIRParametrisation.h"
#include "mir/util/Exceptions.h"


namespace mir {
namespace method {
namespace knn {
namespace pick {


DistanceOrNClosest::DistanceOrNClosest(const param::MIRParametrisation& param) : nClosest_(param) {
    distance_ = 1.;
    param.get("distance", distance_);
    ASSERT(distance_ > 0.);
}


void DistanceOrNClosest::pick(const search::PointSearch& tree, const Point3& p, Pick::neighbours_t& closest) const {
    // TODO: improve k-d tree interface; this is slow because distance might be excessive
    tree.closestWithinRadius(p, distance_, closest);
    if (closest.size() < nClosest_.n()) {
        nClosest_.pick(tree, p, closest);
    }
}


size_t DistanceOrNClosest::n() const {
    return nClosest_.n();
}


bool DistanceOrNClosest::sameAs(const Pick& other) const {
    auto o = dynamic_cast<const DistanceOrNClosest*>(&other);
    return (o != nullptr) && nClosest_.sameAs(o->nClosest_) &&
           eckit::types::is_approximately_equal(distance_, o->distance_);
}


void DistanceOrNClosest::print(std::ostream& out) const {
    out << "DistanceOrNClosest[nclosest=" << nClosest_ << ",distance=" << distance_ << "]";
}


void DistanceOrNClosest::hash(eckit::MD5& h) const {
    h.add("distance-or-nclosest");
    h.add(distance_);
    nClosest_.hash(h);
}


static PickBuilder<DistanceOrNClosest> __pick("distance-or-nclosest");


}  // namespace pick
}  // namespace knn
}  // namespace method
}  // namespace mir
