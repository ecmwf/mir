// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/method/knn/KNearest.h"

#include "mir/method/knn/distance/DistanceWeighting.h"
#include "mir/method/knn/pick/Pick.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/Exceptions.h"


namespace mir::method::knn {


KNearest::KNearest(const param::MIRParametrisation& param) : KNearestNeighbours(param) {

    std::string name = "inverse-distance-weighting-squared";
    param.get("distance-weighting", name);

    distanceWeighting_.reset(distance::DistanceWeightingFactory::build(name, param));
    ASSERT(distanceWeighting_);

    std::string nearestMethod = "nearest-neighbour-with-lowest-index";
    param.get("nearest-method", nearestMethod);
    pick_.reset(pick::PickFactory::build(nearestMethod, param));
}


bool KNearest::sameAs(const Method& other) const {
    const auto* o = dynamic_cast<const KNearest*>(&other);
    return (o != nullptr) && KNearestNeighbours::sameAs(other);
}


const pick::Pick& KNearest::pick() const {
    ASSERT(pick_);
    return *pick_;
}


const distance::DistanceWeighting& KNearest::distanceWeighting() const {
    ASSERT(distanceWeighting_);
    return *distanceWeighting_;
}


const char* KNearest::type() const {
    return "k-nearest";
}


static const MethodBuilder<KNearest> __method1("k-nearest");
static const MethodBuilder<KNearest> __method2("k-nearest-neighbours");
static const MethodBuilder<KNearest> __method3("k-nearest-neighbors");


}  // namespace mir::method::knn
