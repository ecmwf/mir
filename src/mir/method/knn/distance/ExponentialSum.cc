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


#include "mir/method/knn/distance/ExponentialSum.h"

#include <sstream>

#include "eckit/types/FloatCompare.h"
#include "eckit/utils/MD5.h"

#include "mir/param/MIRParametrisation.h"
#include "mir/util/Exceptions.h"


namespace mir {
namespace method {
namespace knn {
namespace distance {


ExponentialSum::ExponentialSum(const param::MIRParametrisation& parametrisation) {
    parametrisation.get("distance-weighting-exponential-tolerance", tolerance_ = 0.);
    ASSERT(tolerance_ >= 0.);

    tolerance2_ = tolerance_ * tolerance_;
    if (eckit::types::is_approximately_equal(tolerance2_, 0.)) {
        tolerance2_ = 0.;
    }
}


void ExponentialSum::operator()(size_t ip, const Point3& point,
                                const std::vector<search::PointSearch::PointValueType>& neighbours,
                                std::vector<WeightMatrix::Triplet>& triplets) const {
    ASSERT(!neighbours.empty());

    triplets.clear();
    triplets.reserve(neighbours.size());

    // calculate neighbour points weights
    auto weight = tolerance2_ > 0. ? [](double d2) { return 1. / d2; }
                                   : [](double d2) { return eckit::types::is_approximately_equal(d2, 0.) ? 1. : 0.; };

    for (const auto& neighbour : neighbours) {
        triplets.emplace_back(ip, neighbour.payload(), weight(Point3::distance2(point, neighbour.point())));
    }
}


bool ExponentialSum::sameAs(const DistanceWeighting& other) const {
    const auto* o = dynamic_cast<const ExponentialSum*>(&other);
    return (o != nullptr) && eckit::types::is_approximately_equal(tolerance_, o->tolerance_);
}


void ExponentialSum::print(std::ostream& out) const {
    out << "ExponentialSum[tolerance=" << tolerance_ << "]";
}


void ExponentialSum::hash(eckit::MD5& h) const {
    std::ostringstream s;
    s << *this;
    h.add(s.str());
}


static const DistanceWeightingBuilder<ExponentialSum> __distance("exponential-sum");


}  // namespace distance
}  // namespace knn
}  // namespace method
}  // namespace mir
