// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/method/knn/pick/DistanceOrNClosest.h"

#include "eckit/log/JSON.h"
#include "eckit/types/FloatCompare.h"
#include "eckit/utils/MD5.h"

#include "mir/param/DefaultParametrisation.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/Exceptions.h"


namespace mir::method::knn::pick {


DistanceOrNClosest::DistanceOrNClosest(const param::MIRParametrisation& param) :
    nClosest_(param), distance_(param::DefaultParametrisation::instance().get_value<double>("distance", param)) {
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
    const auto* o = dynamic_cast<const DistanceOrNClosest*>(&other);
    return (o != nullptr) && nClosest_.sameAs(o->nClosest_) &&
           eckit::types::is_approximately_equal(distance_, o->distance_);
}


void DistanceOrNClosest::json(eckit::JSON& j) const {
    j << type() << "distance-or-nclosest";
    param::DefaultParametrisation::instance().json(j, "distance", distance_);
    param::DefaultParametrisation::instance().json(j, "nclosest", nClosest_.n());
}


void DistanceOrNClosest::print(std::ostream& out) const {
    out << "DistanceOrNClosest[nclosest=" << nClosest_ << ",distance=" << distance_ << "]";
}


void DistanceOrNClosest::hash(eckit::MD5& h) const {
    h.add("distance-or-nclosest");
    h.add(distance_);
    nClosest_.hash(h);
}


static const PickBuilder<DistanceOrNClosest> __pick("distance-or-nclosest");


}  // namespace mir::method::knn::pick
