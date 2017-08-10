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

#include <limits>
#include "eckit/types/FloatCompare.h"
#include "mir/lsm/LandSeaMasks.h"


namespace mir {
namespace method {
namespace distance {


struct Choice {
    explicit Choice(bool sameType, double distance, size_t index) :
        sameType_(size_t(sameType)),
        distance_(distance),
        index_(index) {}
    size_t sameType_;
    double distance_;
    size_t index_;

    bool operator<(const Choice& other) {
    return sameType_ >  other.sameType_ || (
           sameType_ == other.sameType_ && (
           eckit::types::is_strictly_greater    (other.distance_, distance_) || (
           eckit::types::is_approximately_equal (other.distance_, distance_) &&
           index_ < other.index_ )));
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
    Choice choice(false, std::numeric_limits<double>::infinity(), std::numeric_limits<size_t>::max());
    for (auto n : neighbours) {
        ASSERT(n.payload() < imask_.size());
        Choice candidate(omask_[ip] == imask_[n.payload()], eckit::geometry::Point3::distance2(point, n.point()), n.payload());
        if (candidate < choice) {
            choice = candidate;
        }
    }

    ASSERT(choice.index_ < imask_.size());
    size_t jp = choice.index_;

    triplets.assign(1, WeightMatrix::Triplet(ip, jp, 1.));
}


}  // namespace distance
}  // namespace method
}  // namespace mir

