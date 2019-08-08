/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/method/knn/distance/GaussianDistanceWeighting.h"

#include <cmath>

#include "eckit/exception/Exceptions.h"
#include "eckit/types/FloatCompare.h"
#include "eckit/utils/MD5.h"

#include "mir/api/Atlas.h"
#include "mir/param/MIRParametrisation.h"


namespace mir {
namespace method {
namespace knn {
namespace distance {


GaussianDistanceWeighting::GaussianDistanceWeighting(const param::MIRParametrisation& parametrisation) {

    stddev_ = atlas::util::Earth::radius();
    parametrisation.get("distance-weighting-gaussian-stddev", stddev_);
    ASSERT(stddev_ > 0.);

    // exponent factor is used (instead of stddev) to speed weights calculations
    exponentFactor_ = - 1. / (2. * stddev_ * stddev_);
    ASSERT(exponentFactor_ < 0.);
}


void GaussianDistanceWeighting::operator()(
        const Point3& point,
        const neighbours_t& neighbours,
        std::vector<double>& weights) const {

    ASSERT(!neighbours.empty());

    weights.clear();
    weights.reserve(neighbours.size());

    // calculate neighbour points weights, and their total (for normalisation)
    double sum = 0.;
    for (auto& n : neighbours) {
        const double d2 = Point3::distance2(point, n.point());
        const double weight = std::exp(d2 * exponentFactor_);

        weights.emplace_back(weight);
        sum += weight;
    }

    // normalise all weights according to the total
    ASSERT(sum > 0.);
    double invSum = 1. / sum;
    std::for_each(weights.begin(), weights.end(), [=](double& w) { w *= invSum; });
}


bool GaussianDistanceWeighting::sameAs(const DistanceWeighting& other) const {
    auto o = dynamic_cast<const GaussianDistanceWeighting*>(&other);
    return o && eckit::types::is_approximately_equal(stddev_, o->stddev_);
}


void GaussianDistanceWeighting::print(std::ostream& out) const {
    out << "GaussianDistanceWeighting[stddev=" << stddev_ << "]";
}


void GaussianDistanceWeighting::hash(eckit::MD5& h) const {
    std::ostringstream s;
    s << *this;
    h.add(s.str());
}


static DistanceWeightingBuilder<GaussianDistanceWeighting> __distance("gaussian");


}  // namespace distance
}  // namespace knn
}  // namespace method
}  // namespace mir

