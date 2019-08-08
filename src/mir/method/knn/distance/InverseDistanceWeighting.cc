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

#include "eckit/exception/Exceptions.h"
#include "eckit/types/FloatCompare.h"
#include "eckit/utils/MD5.h"

#include "mir/param/MIRParametrisation.h"


namespace mir {
namespace method {
namespace knn {
namespace distance {


InverseDistanceWeighting::InverseDistanceWeighting(const param::MIRParametrisation& parametrisation) {

    power_ = 2.;
    parametrisation.get("distance-weighting-shepard-power", power_);
    ASSERT(power_ >= 0.);

    // half power is used to avoid the Point3::distance extra srqt
    halfPower_ = power_ * 0.5;
    ASSERT(halfPower_ >= 0.);
}


void InverseDistanceWeighting::operator()(
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
        if (eckit::types::is_strictly_greater(d2, 0.)) {

            double weight = 1. / std::pow(d2, halfPower_);
            weights.push_back(weight);
            sum += weight;

        } else {

            // exact match found, use this neighbour only (inverse distance tends to infinity)
            weights.assign(1, 1.);
            return;

        }
    }

    // normalise all weights according to the total
    ASSERT(sum > 0.);
    double invSum = 1. / sum;
    std::for_each(weights.begin(), weights.end(), [=](double& w) { w *= invSum; });
}


bool InverseDistanceWeighting::sameAs(const DistanceWeighting& other) const {
    auto o = dynamic_cast<const InverseDistanceWeighting*>(&other);
    return o && eckit::types::is_approximately_equal(power_, o->power_);
}


void InverseDistanceWeighting::print(std::ostream& out) const {
    out << "InverseDistanceWeighting[power=" << power_ << "]";
}


void InverseDistanceWeighting::hash(eckit::MD5& h) const {
    std::ostringstream s;
    s << *this;
    h.add(s.str());
}


static DistanceWeightingBuilder<InverseDistanceWeighting> __distance1("inverse-distance-weighting");
static DistanceWeightingBuilder<InverseDistanceWeighting> __distance2("shepard");


}  // namespace distance
}  // namespace knn
}  // namespace method
}  // namespace mir

