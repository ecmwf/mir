// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/method/knn/distance/NoDistanceWeighting.h"

#include <sstream>

#include "eckit/log/JSON.h"
#include "eckit/utils/MD5.h"

#include "mir/util/Exceptions.h"


namespace mir::method::knn::distance {


NoDistanceWeighting::NoDistanceWeighting(const param::MIRParametrisation& /*unused*/) {}


void NoDistanceWeighting::operator()(size_t ip, const Point3& /*point*/,
                                     const std::vector<search::PointSearch::PointValueType>& neighbours,
                                     std::vector<WeightMatrix::Triplet>& triplets) const {
    ASSERT(!neighbours.empty());

    triplets.clear();
    triplets.reserve(neighbours.size());

    // average neighbour points
    auto weight = 1. / static_cast<double>(neighbours.size());
    for (const auto& n : neighbours) {
        triplets.emplace_back(ip, n.payload(), weight);
    }
}


bool NoDistanceWeighting::sameAs(const DistanceWeighting& other) const {
    const auto* o = dynamic_cast<const NoDistanceWeighting*>(&other);
    return (o != nullptr);
}


void NoDistanceWeighting::json(eckit::JSON& j) const {
    j << type() << "no";
}


void NoDistanceWeighting::print(std::ostream& out) const {
    out << "NoDistanceWeighting[]";
}


void NoDistanceWeighting::hash(eckit::MD5& h) const {
    std::ostringstream s;
    s << *this;
    h.add(s.str());
}


static const DistanceWeightingBuilder<NoDistanceWeighting> __distance1("no-distance-weighting");
static const DistanceWeightingBuilder<NoDistanceWeighting> __distance2("no");


}  // namespace mir::method::knn::distance
