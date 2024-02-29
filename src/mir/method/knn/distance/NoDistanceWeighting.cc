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
    j.startObject();
    j << "type" << "no";
    j.endObject();
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
