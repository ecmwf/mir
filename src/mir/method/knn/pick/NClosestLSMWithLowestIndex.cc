/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/method/knn/pick/NClosestLSMWithLowestIndex.h"

#include "eckit/exception/Exceptions.h"
#include "eckit/types/FloatCompare.h"
#include "eckit/utils/MD5.h"

#include "mir/lsm/LandSeaMasks.h"
#include "mir/param/MIRParametrisation.h"


namespace mir {
namespace method {
namespace knn {
namespace pick {


NClosestLSMWithLowestIndex::NClosestLSMWithLowestIndex(const param::MIRParametrisation& param,
                                                     const lsm::LandSeaMasks& lsm) :
    PickWithLSM(param, lsm),
    nClosest_(param) {

    param.get("distance-tolerance", distanceTolerance_ = 1.);
    ASSERT(distanceTolerance_ >= 0.);

    distanceTolerance2_ = distanceTolerance_ * distanceTolerance_;
}


void NClosestLSMWithLowestIndex::pick(const search::PointSearch& tree, size_t ip, const eckit::geometry::Point3& point,
                                     Pick::neighbours_t& closest) const {
    nClosest_.pick(tree, ip, point, closest);
    ASSERT(!closest.empty());

    ASSERT(ip < omask_.size());
    bool type(omask_[ip]);

    // choose closest neighbour point with the same output mask value, shortest distance and lowest index
    size_t i = 0;
    ASSERT(closest[i].payload() < imask_.size());

    bool iType = type == imask_[closest[i].payload()];
    double d2  = Point3::distance2(point, closest[i].point());

    for (size_t j = 1; j < closest.size(); ++j) {
        ASSERT(closest[j].payload() < imask_.size());

        bool jType = type == imask_[closest[j].payload()];
        if (iType < jType ||
            eckit::types::is_strictly_greater(d2, Point3::distance2(point, closest[j].point()), distanceTolerance2_) ||
            i > j) {
            i     = j;
            iType = jType;
        }
    }

    closest.assign(1, closest[i]);
}


bool NClosestLSMWithLowestIndex::sameAs(const Pick& other) const {
    auto o = dynamic_cast<const NClosestLSMWithLowestIndex*>(&other);
    return o && nClosest_.sameAs(o->nClosest_) &&
           eckit::types::is_approximately_equal(distanceTolerance_, o->distanceTolerance_) &&
           PickWithLSM::sameAs(other);
}


void NClosestLSMWithLowestIndex::print(std::ostream& out) const {
    out << "NClosestLSMWithLowestIndex[" << nClosest_ << ",distanceTolerance=" << distanceTolerance_ << "]";
}


void NClosestLSMWithLowestIndex::hash(eckit::MD5& h) const {
    h.add("nclosest-lsm-with-lowest-index");
    nClosest_.hash(h);
    h.add(distanceTolerance_);
}


static PickWithLSMBuilder<NClosestLSMWithLowestIndex> __pick("nclosest-lsm-with-lowest-index");


}  // namespace pick
}  // namespace knn
}  // namespace method
}  // namespace mir
