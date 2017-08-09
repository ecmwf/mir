/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/method/distance/InverseDistanceWeightingSquared.h"


namespace mir {
namespace method {
namespace distance {


InverseDistanceWeightingSquared::InverseDistanceWeightingSquared(const param::MIRParametrisation&) {
}


void InverseDistanceWeightingSquared::operator()(
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

        weights[j] = 1. / (1. + d2);
        sum += weights[j];

    }

    ASSERT(sum > 0.);

    // normalise all weights according to the total, and set sparse matrix triplets
    for (size_t i = 0; i < nbPoints; ++i) {
        size_t jp = neighbours[i].payload();
        triplets.push_back(WeightMatrix::Triplet(ip, jp, weights[i] / sum));
    }
}


static DistanceWeightingBuilder<InverseDistanceWeightingSquared> __distance("inverse-distance-weighting-squared");


}  // namespace distance
}  // namespace method
}  // namespace mir

