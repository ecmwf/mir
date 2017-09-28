/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/method/distance/NearestLSM.h"

#include "mir/lsm/LandSeaMasks.h"


namespace mir {
namespace method {
namespace distance {


NearestLSM::NearestLSM(const param::MIRParametrisation&, const lsm::LandSeaMasks& landSeaMasks) :
    imask_(landSeaMasks.inputMask()),
    omask_(landSeaMasks.outputMask()) {
    ASSERT(!imask_.empty());
    ASSERT(!omask_.empty());
}


void NearestLSM::operator()(
        size_t ip,
        const eckit::geometry::Point3&,
        const std::vector<util::PointSearch::PointValueType>& neighbours,
        std::vector<WeightMatrix::Triplet>& triplets ) const {

    ASSERT(!neighbours.empty());
    ASSERT(ip < omask_.size());

    // choose closest neighbour point with the same output mask value
    size_t jp = 0;
    bool foundSameType = false;

    for (auto p : neighbours) {
        jp = p.payload();
        ASSERT(jp < imask_.size());

        if (omask_[ip] == imask_[jp]) {
            foundSameType = true;
            break;
        }
    }

    if (!foundSameType) {
        jp = neighbours.front().payload();
    }

    triplets.assign(1, WeightMatrix::Triplet(ip, jp, 1.));
}


}  // namespace distance
}  // namespace method
}  // namespace mir

