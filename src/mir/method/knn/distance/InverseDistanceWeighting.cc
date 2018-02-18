/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/method/knn/distance/InverseDistanceWeighting.h"

#include "eckit/types/FloatCompare.h"


namespace mir {
namespace method {
namespace knn {
namespace distance {


InverseDistanceWeighting::InverseDistanceWeighting(const param::MIRParametrisation&) {
}


void InverseDistanceWeighting::operator()(
        size_t ip,
        const eckit::geometry::Point3& point,
        const std::vector<util::PointSearch::PointValueType>& neighbours,
        std::vector<WeightMatrix::Triplet>& triplets ) const {

    const size_t nbPoints = neighbours.size();
    ASSERT(nbPoints);

    triplets.clear();
    triplets.reserve(nbPoints);

    // calculate neighbour points weights, and their total (for normalisation)
    std::vector<double> weights(nbPoints);
    double sum = 0.;
    for (size_t j = 0; j < nbPoints; ++j) {
        const double d2 = eckit::geometry::Point3::distance2(point, neighbours[j].point());
        if (eckit::types::is_strictly_greater(d2, 0.)) {

            weights[j] = 1. / d2;
            sum += weights[j];

        } else {

            // exact match found, use this neighbour only
            triplets.assign(1, WeightMatrix::Triplet(ip, neighbours[j].payload(), 1.));
            return;

        }
    }

    ASSERT(sum > 0.);

    // normalise all weights according to the total, and set sparse matrix triplets
    for (size_t j = 0; j < nbPoints; ++j) {
        size_t jp = neighbours[j].payload();
        triplets.push_back(WeightMatrix::Triplet(ip, jp, weights[j] / sum));
    }
}


static DistanceWeightingBuilder<InverseDistanceWeighting> __distance("inverse-distance-weighting");


}  // namespace distance
}  // namespace knn
}  // namespace method
}  // namespace mir

