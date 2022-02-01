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


#include "mir/method/knn/distance/GaussianDistanceWeighting.h"

#include <cmath>
#include <sstream>

#include "eckit/types/FloatCompare.h"
#include "eckit/utils/MD5.h"

#include "mir/param/MIRParametrisation.h"
#include "mir/util/Atlas.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Types.h"


namespace mir {
namespace method {
namespace knn {
namespace distance {


GaussianDistanceWeighting::GaussianDistanceWeighting(const param::MIRParametrisation& parametrisation) {

    stddev_ = util::Earth::radius();
    parametrisation.get("distance-weighting-gaussian-stddev", stddev_);
    ASSERT(stddev_ > 0.);

    // exponent factor is used (instead of stddev) to speed weights calculations
    exponentFactor_ = -1. / (2. * stddev_ * stddev_);
    ASSERT(exponentFactor_ < 0.);
}


void GaussianDistanceWeighting::operator()(size_t ip, const Point3& point,
                                           const std::vector<search::PointSearch::PointValueType>& neighbours,
                                           std::vector<WeightMatrix::Triplet>& triplets) const {

    ASSERT(!neighbours.empty());

    triplets.clear();
    triplets.reserve(neighbours.size());

    // calculate neighbour points weights, and their total (for normalisation)
    double sum = 0.;
    for (const auto& n : neighbours) {
        const double d2     = Point3::distance2(point, n.point());
        const double weight = std::exp(d2 * exponentFactor_);

        triplets.emplace_back(WeightMatrix::Triplet(ip, n.payload(), weight));
        sum += weight;
    }

    // normalise all weights according to the total weights sum
    ASSERT(sum > 0.);
    for (auto& t : triplets) {
        t.value() /= sum;
    }
}


bool GaussianDistanceWeighting::sameAs(const DistanceWeighting& other) const {
    const auto* o = dynamic_cast<const GaussianDistanceWeighting*>(&other);
    return (o != nullptr) && eckit::types::is_approximately_equal(stddev_, o->stddev_);
}


void GaussianDistanceWeighting::print(std::ostream& out) const {
    out << "GaussianDistanceWeighting[stddev=" << stddev_ << "]";
}


void GaussianDistanceWeighting::hash(eckit::MD5& h) const {
    std::ostringstream s;
    s << *this;
    h.add(s.str());
}


static const DistanceWeightingBuilder<GaussianDistanceWeighting> __distance("gaussian");


}  // namespace distance
}  // namespace knn
}  // namespace method
}  // namespace mir
