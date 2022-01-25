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


#include "mir/method/knn/distance/Cressman.h"

#include <sstream>

#include "eckit/types/FloatCompare.h"
#include "eckit/utils/MD5.h"

#include "mir/param/MIRParametrisation.h"
#include "mir/util/Exceptions.h"


namespace mir {
namespace method {
namespace knn {
namespace distance {


Cressman::Cressman(const param::MIRParametrisation& parametrisation) {
    parametrisation.get("cressman-model-extension-power", power_ = 1.);
    ASSERT(power_ >= 1.);

    parametrisation.get("distance", r_ = 1.);
    ASSERT(r_ >= 0.);

    r2_ = r_ * r_;
}


void Cressman::operator()(size_t ip, const Point3& point,
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
        weights[j] = d2 < r2_ ? std::pow((d2 - r2_) / (d2 + r2_), power_) : 0.;
        sum += weights[j];
    }

    // normalise all weights according to the total, and set sparse matrix triplets
    if (eckit::types::is_strictly_greater(sum, 0.)) {
        for (size_t j = 0; j < nbPoints; ++j) {
            size_t jp = neighbours[j].payload();
            triplets.emplace_back(WeightMatrix::Triplet(ip, jp, weights[j] / sum));
        }
    }
}


bool Cressman::sameAs(const DistanceWeighting& other) const {
    auto o = dynamic_cast<const Cressman*>(&other);
    return (o != nullptr) && eckit::types::is_approximately_equal(r_, o->r_);
}


void Cressman::print(std::ostream& out) const {
    out << "Cressman[radius=" << r_ << "]";
}


void Cressman::hash(eckit::MD5& h) const {
    std::ostringstream s;
    s << *this;
    h.add(s.str());
}


static const DistanceWeightingBuilder<Cressman> __distance("cressman");


}  // namespace distance
}  // namespace knn
}  // namespace method
}  // namespace mir
