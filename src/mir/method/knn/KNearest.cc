/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Tiago Quintino
/// @author Pedro Maciel
/// @date May 2015


#include "mir/method/knn/KNearest.h"

#include "mir/param/MIRParametrisation.h"
#include "mir/method/knn/distance/DistanceWeighting.h"


namespace mir {
namespace method {
namespace knn {


KNearest::KNearest(const param::MIRParametrisation& param) : KNearestNeighbours(param) {

    std::string name = "inverse-distance-weighting-squared";
    param.get("distance-weighting", name);

    distanceWeighting_.reset(distance::DistanceWeightingFactory::build(name, param));
    ASSERT(distanceWeighting_);
}


KNearest::~KNearest() {
}


bool KNearest::sameAs(const Method& other) const {
    const KNearest* o = dynamic_cast<const KNearest*>(&other);
    return o && KNearestNeighbours::sameAs(other);
}


const distance::DistanceWeighting& KNearest::distanceWeighting() const {
    ASSERT(distanceWeighting_);
    return *distanceWeighting_;
}


const char* KNearest::name() const {
    return "k-nearest";
}


namespace {
static MethodBuilder< KNearest > __method("k-nearest");
}


}  // namespace knn
}  // namespace method
}  // namespace mir

