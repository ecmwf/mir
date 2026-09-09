// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/method/knn/pick/NClosest.h"

#include "eckit/log/JSON.h"
#include "eckit/utils/MD5.h"

#include "mir/param/DefaultParametrisation.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/Exceptions.h"


namespace mir::method::knn::pick {


NClosest::NClosest(const param::MIRParametrisation& param) :
    nClosest_(param::DefaultParametrisation::instance().get_value<size_t>("nclosest", param)) {
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


void NClosest::json(eckit::JSON& j) const {
    j << type() << "nclosest";
    param::DefaultParametrisation::instance().json(j, "nclosest", nClosest_);
}


void NClosest::print(std::ostream& out) const {
    out << "NClosest[" << nClosest_ << "]";
}


void NClosest::hash(eckit::MD5& h) const {
    h.add(nClosest_);
}


static const PickBuilder<NClosest> __pick("nclosest");


}  // namespace mir::method::knn::pick
