/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/method/knn/distance/InverseDistanceWeightingSquared.h"

#include "eckit/exception/Exceptions.h"
#include "eckit/utils/MD5.h"


namespace mir {
namespace method {
namespace knn {
namespace distance {


InverseDistanceWeightingSquared::InverseDistanceWeightingSquared(const param::MIRParametrisation&) {
}


void InverseDistanceWeightingSquared::operator()(
        const Point3& point,
        const neighbours_t& neighbours,
        std::vector<double>& weights ) const {

    const size_t nbPoints = neighbours.size();
    ASSERT(nbPoints);

    weights.clear();
    weights.reserve(nbPoints);

    // calculate neighbour points weights, and their total (for normalisation)
    double sum = 0.;
    for (size_t j = 0; j < nbPoints; ++j) {
        const double d2 = Point3::distance2(point, neighbours[j].point());

        weights[j] = 1. / (1. + d2);
        sum += weights[j];
    }

    // normalise all weights according to the total
    ASSERT(sum > 0.);
    double invSum = 1. / sum;
    std::for_each(weights.begin(), weights.end(), [=](double& w) { w *= invSum; });
}


bool InverseDistanceWeightingSquared::sameAs(const DistanceWeighting& other) const {
    auto o = dynamic_cast<const InverseDistanceWeightingSquared*>(&other);
    return o;
}


void InverseDistanceWeightingSquared::print(std::ostream& out) const {
    out << "InverseDistanceWeightingSquared[]";
}


void InverseDistanceWeightingSquared::hash(eckit::MD5& h) const {
    std::ostringstream s;
    s << *this;
    h.add(s.str());
}


static DistanceWeightingBuilder<InverseDistanceWeightingSquared> __distance("inverse-distance-weighting-squared");


}  // namespace distance
}  // namespace knn
}  // namespace method
}  // namespace mir

