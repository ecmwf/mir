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


#include "mir/method/knn/KNearest.h"

#include "mir/method/knn/distance/DistanceWeighting.h"
#include "mir/method/knn/pick/Pick.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/Exceptions.h"


namespace mir {
namespace method {
namespace knn {


KNearest::KNearest(const param::MIRParametrisation& param) : KNearestNeighbours(param) {

    std::string name = "inverse-distance-weighting-squared";
    param.get("distance-weighting", name);

    distanceWeighting_.reset(distance::DistanceWeightingFactory::build(name, param));
    ASSERT(distanceWeighting_);

    std::string nearestMethod = "nearest-neighbour-with-lowest-index";
    param.get("nearest-method", nearestMethod);
    pick_.reset(pick::PickFactory::build(nearestMethod, param));
}


KNearest::~KNearest() = default;


bool KNearest::sameAs(const Method& other) const {
    auto o = dynamic_cast<const KNearest*>(&other);
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


const char* KNearest::name() const {
    return "k-nearest";
}


static MethodBuilder<KNearest> __method1("k-nearest");
static MethodBuilder<KNearest> __method2("k-nearest-neighbours");
static MethodBuilder<KNearest> __method3("k-nearest-neighbors");


}  // namespace knn
}  // namespace method
}  // namespace mir
