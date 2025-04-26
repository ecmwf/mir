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


#include "mir/method/knn/pick/DistanceAndNClosest.h"

#include "eckit/log/JSON.h"
#include "eckit/types/FloatCompare.h"
#include "eckit/utils/MD5.h"

#include "mir/param/MIRParametrisation.h"
#include "mir/util/Exceptions.h"


namespace mir::method::knn::pick {


DistanceAndNClosest::DistanceAndNClosest(const param::MIRParametrisation& param) : nClosest_(param) {
    distance_ = 1.;
    param.get("distance", distance_);
    ASSERT(distance_ > 0.);
}


void DistanceAndNClosest::pick(const search::PointSearch& tree, const PointXYZ& p, Pick::neighbours_t& closest) const {
    // TODO: improve k-d tree interface; this is slow because distance might be excessive
    tree.closestWithinRadius(p, distance_, closest);
    if (closest.size() > nClosest_.n()) {
        nClosest_.pick(tree, p, closest);
    }
}


size_t DistanceAndNClosest::n() const {
    return nClosest_.n();
}


bool DistanceAndNClosest::sameAs(const Pick& other) const {
    const auto* o = dynamic_cast<const DistanceAndNClosest*>(&other);
    return (o != nullptr) && nClosest_.sameAs(o->nClosest_) &&
           eckit::types::is_approximately_equal(distance_, o->distance_);
}


void DistanceAndNClosest::json(eckit::JSON& j) const {
    j.startObject();
    j << "type"
      << "distance-and-nclosest";
    j << "nclosest" << nClosest_;
    j << "distance" << distance_;
    j.endObject();
}


void DistanceAndNClosest::print(std::ostream& out) const {
    out << "DistanceAndNClosest[nclosest=" << nClosest_ << ",distance=" << distance_ << "]";
}


void DistanceAndNClosest::hash(eckit::MD5& h) const {
    h.add("distance-and-nclosest");
    h.add(distance_);
    nClosest_.hash(h);
}


static const PickBuilder<DistanceAndNClosest> __pick("distance-and-nclosest");


}  // namespace mir::method::knn::pick
