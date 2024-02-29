/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


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
    j.startObject();
    j << "type" << "inverse-distance-weighting-squared";
    j.endObject();
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
