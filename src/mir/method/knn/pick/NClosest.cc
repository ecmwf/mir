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


#include "mir/method/knn/pick/NClosest.h"

#include "eckit/utils/MD5.h"

#include "mir/param/MIRParametrisation.h"
#include "mir/util/Exceptions.h"


namespace mir::method::knn::pick {


NClosest::NClosest(const param::MIRParametrisation& param) {
    nClosest_ = 4;
    param.get("nclosest", nClosest_);
    ASSERT(nClosest_ > 0);
}


void NClosest::pick(const search::PointSearch& tree, const Point3& p, Pick::neighbours_t& closest) const {
    tree.closestNPoints(p, nClosest_, closest);
    ASSERT(closest.size() == nClosest_);
}


size_t NClosest::n() const {
    return nClosest_;
}


bool NClosest::sameAs(const Pick& other) const {
    const auto* o = dynamic_cast<const NClosest*>(&other);
    return (o != nullptr) && nClosest_ == o->nClosest_;
}


void NClosest::print(std::ostream& out) const {
    out << "NClosest[" << nClosest_ << "]";
}


void NClosest::hash(eckit::MD5& h) const {
    h.add(nClosest_);
}


static const PickBuilder<NClosest> __pick("nclosest");


}  // namespace mir::method::knn::pick
