/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/method/knn/pick/NClosestLSM.h"

#include "eckit/exception/Exceptions.h"
#include "eckit/utils/MD5.h"

#include "mir/lsm/LandSeaMasks.h"


namespace mir {
namespace method {
namespace knn {
namespace pick {


NClosestLSM::NClosestLSM(const param::MIRParametrisation& param, const lsm::LandSeaMasks& lsm) :
    PickWithLSM(param, lsm),
    nClosest_(param) {}


void NClosestLSM::pick(const search::PointSearch& tree, size_t ip, const Point3& point, neighbours_t& closest) const {
    ASSERT(ip < omask_.size());

    nClosest_.pick(tree, ip, point, closest);
    ASSERT(!closest.empty());

    // choose closest neighbour point with the same output mask value
    for (auto& n : closest) {
        size_t jp = n.payload();
        ASSERT(jp < imask_.size());

        if (omask_[ip] == imask_[jp]) {
            closest.assign(1, n);
            break;
        }
    }

    closest.assign(1, closest[0]);
}


bool NClosestLSM::sameAs(const Pick& other) const {
    auto o = dynamic_cast<const NClosestLSM*>(&other);
    return o && nClosest_.sameAs(o->nClosest_) && PickWithLSM::sameAs(other);
}


void NClosestLSM::print(std::ostream& out) const {
    out << "NClosestLSM[nClosest=" << nClosest_ << "]";
}


void NClosestLSM::hash(eckit::MD5& h) const {
    h.add("nclosest-lsm");
    nClosest_.hash(h);
}


static PickWithLSMBuilder<NClosestLSM> __pick("nclosest-lsm");


}  // namespace pick
}  // namespace knn
}  // namespace method
}  // namespace mir
