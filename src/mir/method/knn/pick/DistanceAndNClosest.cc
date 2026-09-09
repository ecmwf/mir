// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/method/knn/pick/DistanceAndNClosest.h"

#include "eckit/log/JSON.h"
#include "eckit/types/FloatCompare.h"
#include "eckit/utils/MD5.h"

#include "mir/param/DefaultParametrisation.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/Exceptions.h"


namespace mir::method::knn::pick {


DistanceAndNClosest::DistanceAndNClosest(const param::MIRParametrisation& param) :
    nClosest_(param), distance_(param::DefaultParametrisation::instance().get_value<double>("distance", param)) {
    ASSERT(distance_ > 0.);
}


void DistanceAndNClosest::pick(const search::PointSearch& tree, const Point3& p, Pick::neighbours_t& closest) const {
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
    j << type() << "distance-and-nclosest";
    param::DefaultParametrisation::instance().json(j, "distance", distance_);
    param::DefaultParametrisation::instance().json(j, "nclosest", nClosest_.n());
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
