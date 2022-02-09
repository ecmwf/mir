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


#include "mir/method/knn/distance/ExponentialMean.h"

#include <cmath>
#include <sstream>

#include "eckit/types/FloatCompare.h"
#include "eckit/utils/MD5.h"

#include "mir/param/MIRParametrisation.h"
#include "mir/util/Exceptions.h"


namespace mir {
namespace method {
namespace knn {
namespace distance {


ExponentialMean::ExponentialMean(const param::MIRParametrisation& parametrisation) {
    parametrisation.get("distance-weighting-exponential-tolerance", tolerance_ = 0.);
    ASSERT(tolerance_ >= 0.);

    tolerance2_ = tolerance_ * tolerance_;
    if (eckit::types::is_approximately_equal(tolerance2_, 0.)) {
        tolerance2_ = 0.;
    }
}


void ExponentialMean::operator()(size_t ip, const Point3& point,
                                 const std::vector<search::PointSearch::PointValueType>& neighbours,
                                 std::vector<WeightMatrix::Triplet>& triplets) const {
    const size_t nbPoints = neighbours.size();
    ASSERT(0 < nbPoints);

    triplets.clear();
    triplets.reserve(nbPoints);

    // calculate neighbour points weights, and their total (for normalisation)
    std::vector<double> weights(nbPoints);
    double sum = 0.;
    for (size_t j = 0; j < nbPoints; ++j) {
        auto d2    = Point3::distance2(point, neighbours[j].point());
        weights[j] = tolerance2_ > 0.                               ? std::exp(-d2 / tolerance2_)
                     : eckit::types::is_approximately_equal(d2, 0.) ? 1.
                                                                    : 0.;
        sum += weights[j];
    }

    // normalise all weights according to the total
    if (eckit::types::is_strictly_greater(sum, 0.)) {
        for (size_t j = 0; j < nbPoints; ++j) {
            size_t jp = neighbours[j].payload();
            triplets.emplace_back(WeightMatrix::Triplet(ip, jp, weights[j] / sum));
        }
    }
}


bool ExponentialMean::sameAs(const DistanceWeighting& other) const {
    const auto* o = dynamic_cast<const ExponentialMean*>(&other);
    return (o != nullptr) && eckit::types::is_approximately_equal(tolerance_, o->tolerance_);
}


void ExponentialMean::print(std::ostream& out) const {
    out << "ExponentialMean[tolerance=" << tolerance_ << "]";
}


void ExponentialMean::hash(eckit::MD5& h) const {
    std::ostringstream s;
    s << *this;
    h.add(s.str());
}


static const DistanceWeightingBuilder<ExponentialMean> __distance("exponential-mean");


}  // namespace distance
}  // namespace knn
}  // namespace method
}  // namespace mir
