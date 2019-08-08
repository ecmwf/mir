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

#include "eckit/utils/MD5.h"

#include "mir/method/knn/distance/DistanceWeighting.h"
#include "mir/param/MIRParametrisation.h"


namespace mir {
namespace method {
namespace knn {


KNearest::KNearest(const param::MIRParametrisation& param) : KNearestNeighbours(param) {

    std::string nearestMethod;
    param.get("nearest-method", nearestMethod = "nclosest-or-nearest");

    pick_.reset(pick::PickFactory::build(nearestMethod, param));
    ASSERT(pick_);
}

void KNearest::assemble(util::MIRStatistics& stats, WeightMatrix& W, const repres::Representation& in,
                        const repres::Representation& out) const {
    ASSERT(pick_);
    assembleCustomised(stats, W, in, out, pick(), distanceWeighting());
}


KNearest::~KNearest() = default;


bool KNearest::sameAs(const Method& other) const {
    auto o = dynamic_cast<const KNearest*>(&other);
    return o && pick().sameAs(o->pick()) && KNearestNeighbours::sameAs(other);
}


void KNearest::print(std::ostream& out) const {
    ASSERT(pick_);

    out << "KNearest[";
    MethodWeighted::print(out);
    out << ",nearestMethod=" << *pick_ << ",distanceWeighting=" << distanceWeighting() << "]";
}


const pick::Pick& KNearest::pick() const {
    ASSERT(pick_);
    return *pick_;
}


const char* KNearest::name() const {
    return "k-nearest";
}


namespace {
static MethodBuilder<KNearest> __method("k-nearest");
}


}  // namespace knn
}  // namespace method
}  // namespace mir
