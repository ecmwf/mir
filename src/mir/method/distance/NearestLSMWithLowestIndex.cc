/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/method/distance/NearestLSMWithLowestIndex.h"

#include <set>
#include "eckit/types/FloatCompare.h"
#include "mir/lsm/LandSeaMasks.h"


namespace mir {
namespace method {
namespace distance {


struct SortableNeighbour {
    explicit SortableNeighbour(bool sameType, double distance, size_t index) :
        distance_(distance),
        index_(index),
        sameType_(size_t(sameType)) {}
    const double distance_;
    const size_t index_;
    const size_t sameType_;
};


struct SortBySameTypeThenDistanceThenIndex {
    bool operator() (const SortableNeighbour& lhs, const SortableNeighbour& rhs) const {
        return lhs.sameType_ >  rhs.sameType_ || (
               lhs.sameType_ == rhs.sameType_ && (
               eckit::types::is_strictly_greater    (rhs.distance_, lhs.distance_) || (
               eckit::types::is_approximately_equal (rhs.distance_, lhs.distance_) &&
               lhs.index_ < rhs.index_ )));
    }
};


NearestLSMWithLowestIndex::NearestLSMWithLowestIndex(const param::MIRParametrisation&, const lsm::LandSeaMasks& landSeaMasks) :
    imask_(landSeaMasks.inputMask()),
    omask_(landSeaMasks.outputMask()) {
    ASSERT(!imask_.empty());
    ASSERT(!omask_.empty());
}


void NearestLSMWithLowestIndex::operator()(
        size_t ip,
        const eckit::geometry::Point3& point,
        const std::vector<util::PointSearch::PointValueType>& neighbours,
        std::vector<WeightMatrix::Triplet>& triplets ) const {

    ASSERT(!neighbours.empty());
    ASSERT(ip < omask_.size());

    // choose closest neighbour point with the same output mask value, smallest distance and lowest index
    std::set< SortableNeighbour, SortBySameTypeThenDistanceThenIndex > neighboursSorted;
    for (auto n : neighbours) {
        ASSERT(n.payload() < imask_.size());
        neighboursSorted.insert(SortableNeighbour(
                     omask_[ip] == imask_[n.payload()],
                     eckit::geometry::Point3::distance2(point, n.point()),
                     n.payload()
                     ));
    }

    ASSERT(!neighboursSorted.empty());
    size_t jp = neighboursSorted.begin()->index_;

    triplets.assign(1, WeightMatrix::Triplet(ip, jp, 1.));
}


}  // namespace distance
}  // namespace method
}  // namespace mir

