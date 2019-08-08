/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/method/knn/pick/NClosestWithLowestIndex.h"

#include "eckit/exception/Exceptions.h"
#include "eckit/types/FloatCompare.h"
#include "eckit/utils/MD5.h"

#include "mir/param/MIRParametrisation.h"
#include "mir/method/knn/pick/PickWithLSM.h"


namespace mir {
namespace method {
namespace knn {
namespace pick {


NClosestWithLowestIndex::NClosestWithLowestIndex(const param::MIRParametrisation& param) : nClosest_(param) {

    param.get("distance-tolerance", distanceTolerance_ = 1.);
    ASSERT(distanceTolerance_ >= 0.);

    distanceTolerance2_ = distanceTolerance_ * distanceTolerance_;
}


void NClosestWithLowestIndex::pick(const search::PointSearch& tree, size_t ip, const eckit::geometry::Point3& point,
                                  Pick::neighbours_t& closest) const {
    nClosest_.pick(tree, ip, point, closest);
    ASSERT(!closest.empty());

    // choose closest neighbour point with the lowest index (payload)
    size_t i        = 0;
    const double d2 = Point3::distance2(point, closest.front().point());

    for (size_t j = 1; j < closest.size(); ++j) {
        if (eckit::types::is_strictly_greater(Point3::distance2(point, closest[j].point()), d2, distanceTolerance2_)) {
            break;
        }
        if (i > closest[j].payload()) {
            i = j;
        }
    }

    closest.assign(1, closest[i]);
}


size_t NClosestWithLowestIndex::n() const {
    return nClosest_.n();
}


bool NClosestWithLowestIndex::sameAs(const Pick& other) const {
    auto o = dynamic_cast<const NClosestWithLowestIndex*>(&other);
    return o && nClosest_.sameAs(o->nClosest_) &&
           eckit::types::is_approximately_equal(distanceTolerance_, o->distanceTolerance_);
}


void NClosestWithLowestIndex::print(std::ostream& out) const {
    out << "NClosestWithLowestIndex[" << nClosest_ << ",distanceTolerance=" << distanceTolerance_ << "]";
}


void NClosestWithLowestIndex::hash(eckit::MD5& h) const {
    h.add("nclosest-with-lowest-index");
    h.add(nClosest_);
    h.add(distanceTolerance_);
}


static PickBuilder<NClosestWithLowestIndex> __pick("nclosest-with-lowest-index");


}  // namespace pick
}  // namespace knn
}  // namespace method
}  // namespace mir
