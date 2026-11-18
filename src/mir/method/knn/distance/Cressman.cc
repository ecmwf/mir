// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/method/knn/distance/Cressman.h"

#include <sstream>

#include "eckit/log/JSON.h"
#include "eckit/types/FloatCompare.h"
#include "eckit/utils/MD5.h"

#include "mir/param/DefaultParametrisation.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/Exceptions.h"


namespace mir::method::knn::distance {


Cressman::Cressman(const param::MIRParametrisation& param) :
    r_(param::DefaultParametrisation::instance().get_value<double>("distance", param)), r2_(r_ * r_) {
    param.get("cressman-model-extension-power", power_ = 1.);
    ASSERT(power_ >= 1.);

    ASSERT(r_ >= 0.);
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
        weights[j] = d2 < r2_ ? std::pow((r2_ - d2) / (r2_ + d2), power_) : 0.;
        sum += weights[j];
    }

    // no neighbours within the radius: the nearest neighbour (the first), so every output point is interpolated
    if (!eckit::types::is_strictly_greater(sum, 0.)) {
        triplets.emplace_back(ip, neighbours.front().payload(), 1.);
        return;
    }

    // normalise all weights according to the total, and set sparse matrix triplets
    for (size_t j = 0; j < nbPoints; ++j) {
        size_t jp = neighbours[j].payload();
        triplets.emplace_back(ip, jp, weights[j] / sum);
    }
}


bool Cressman::sameAs(const DistanceWeighting& other) const {
    const auto* o = dynamic_cast<const Cressman*>(&other);
    return (o != nullptr) && eckit::types::is_approximately_equal(r_, o->r_) &&
           eckit::types::is_approximately_equal(power_, o->power_);
}


void Cressman::json(eckit::JSON& j) const {
    j << type() << "cressman";
    param::DefaultParametrisation::instance().json(j, "distance", r_);

    if (!eckit::types::is_approximately_equal(power_, 1.)) {
        j << "cressman-model-extension-power" << power_;
    }
}


void Cressman::print(std::ostream& out) const {
    out << "Cressman[radius=" << r_;
    if (!eckit::types::is_approximately_equal(power_, 1.)) {
        out << ",power=" << power_;
    }
    out << "]";
}


void Cressman::hash(eckit::MD5& h) const {
    std::ostringstream s;
    s << *this;
    h.add(s.str());
}


static const DistanceWeightingBuilder<Cressman> __distance("cressman");


}  // namespace mir::method::knn::distance
