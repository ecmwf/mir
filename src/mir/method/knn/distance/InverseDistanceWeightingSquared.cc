// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/method/knn/distance/InverseDistanceWeightingSquared.h"

#include <sstream>

#include "eckit/log/JSON.h"
#include "eckit/utils/MD5.h"

#include "mir/util/Exceptions.h"


namespace mir::method::knn::distance {


InverseDistanceWeightingSquared::InverseDistanceWeightingSquared(const param::MIRParametrisation& /*unused*/) {}


void InverseDistanceWeightingSquared::operator()(size_t ip, const Point3& point,
                                                 const std::vector<search::PointSearch::PointValueType>& neighbours,
                                                 std::vector<WeightMatrix::Triplet>& triplets) const {

    const size_t nbPoints = neighbours.size();
    ASSERT(nbPoints);

    triplets.clear();
    triplets.reserve(nbPoints);

    // calculate neighbour points weights, and their total (for normalisation)
    std::vector<double> weights(nbPoints);
    double sum = 0.;
    for (size_t j = 0; j < nbPoints; ++j) {
        const double d2 = Point3::distance2(point, neighbours[j].point());

        weights[j] = 1. / (1. + d2);
        sum += weights[j];
    }

    ASSERT(sum > 0.);

    // normalise all weights according to the total, and set sparse matrix triplets
    for (size_t i = 0; i < nbPoints; ++i) {
        size_t jp = neighbours[i].payload();
        triplets.emplace_back(ip, jp, weights[i] / sum);
    }
}


bool InverseDistanceWeightingSquared::sameAs(const DistanceWeighting& other) const {
    const auto* o = dynamic_cast<const InverseDistanceWeightingSquared*>(&other);
    return (o != nullptr);
}


void InverseDistanceWeightingSquared::json(eckit::JSON& j) const {
    j << type() << "inverse-distance-weighting-squared";
}


void InverseDistanceWeightingSquared::print(std::ostream& out) const {
    out << "InverseDistanceWeightingSquared[]";
}


void InverseDistanceWeightingSquared::hash(eckit::MD5& h) const {
    std::ostringstream s;
    s << *this;
    h.add(s.str());
}


static const DistanceWeightingBuilder<InverseDistanceWeightingSquared> __distance("inverse-distance-weighting-squared");


}  // namespace mir::method::knn::distance
