/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/method/distance/NearestNeighbourWithLowestIndex.h"

#include "eckit/types/FloatCompare.h"


namespace mir {
namespace method {
namespace distance {


NearestNeighbourWithLowestIndex::NearestNeighbourWithLowestIndex(const param::MIRParametrisation&) {
}


void NearestNeighbourWithLowestIndex::operator()(
        size_t ip,
        const eckit::geometry::Point3& point,
        const std::vector<util::PointSearch::PointValueType>& neighbours,
        std::vector<WeightMatrix::Triplet>& triplets ) const {

    const size_t nbPoints = neighbours.size();
    ASSERT(nbPoints);

    // choose closest neighbour point with the lowest index (payload)
    size_t jp = neighbours.front().payload();
    const double d2 = eckit::geometry::Point3::distance2(point, neighbours.front().point());

    for (size_t j = 1; j < nbPoints; ++j) {
        if (eckit::types::is_strictly_greater(eckit::geometry::Point3::distance2(point, neighbours[j].point()), d2)) {
            break;
        }
        if (jp > neighbours[j].payload()) {
            jp = neighbours[j].payload();
        }
    }

    triplets.assign(1, WeightMatrix::Triplet(ip, jp, 1.));
}


static DistanceWeightingBuilder<NearestNeighbourWithLowestIndex> __distance("nearest-neighbour-with-lowest-index");


}  // namespace distance
}  // namespace method
}  // namespace mir

