// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/method/knn/NearestNeighbour.h"

#include "mir/method/knn/pick/Pick.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/Exceptions.h"


namespace mir::method::knn {


NearestNeighbour::NearestNeighbour(const param::MIRParametrisation& param) :
    KNearestNeighbours(param), distanceWeighting_(param) {

    std::string nearestMethod = "nearest-neighbour-with-lowest-index";
    param.get("nearest-method", nearestMethod);
    pick_.reset(pick::PickFactory::build(nearestMethod, param));
}


bool NearestNeighbour::sameAs(const Method& other) const {
    const auto* o = dynamic_cast<const NearestNeighbour*>(&other);
    return (o != nullptr) && KNearestNeighbours::sameAs(other);
}


const pick::Pick& NearestNeighbour::pick() const {
    ASSERT(pick_);
    return *pick_;
}


const distance::DistanceWeighting& NearestNeighbour::distanceWeighting() const {
    return distanceWeighting_;
}


const char* NearestNeighbour::type() const {
    return "nearest-neighbour";
}


static const MethodBuilder<NearestNeighbour> __method1("nearest-neighbour");
static const MethodBuilder<NearestNeighbour> __method2("nearest-neighbor");  // For the americans
static const MethodBuilder<NearestNeighbour> __method3("nn");                // For the lazy


}  // namespace mir::method::knn
