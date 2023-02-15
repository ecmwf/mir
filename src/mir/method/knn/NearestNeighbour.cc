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


NearestNeighbour::~NearestNeighbour() = default;


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


const char* NearestNeighbour::name() const {
    return "nearest-neighbour";
}


static const MethodBuilder<NearestNeighbour> __method1("nearest-neighbour");
static const MethodBuilder<NearestNeighbour> __method2("nearest-neighbor");  // For the americans
static const MethodBuilder<NearestNeighbour> __method3("nn");                // For the lazy


}  // namespace mir::method::knn
