// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/method/knn/distance/NearestNeighbour.h"

#include <sstream>

#include "eckit/log/JSON.h"
#include "eckit/utils/MD5.h"

#include "mir/util/Exceptions.h"


namespace mir::method::knn::distance {


NearestNeighbour::NearestNeighbour(const param::MIRParametrisation& /*unused*/) {}


void NearestNeighbour::operator()(size_t ip, const Point3& /*point*/,
                                  const std::vector<search::PointSearch::PointValueType>& neighbours,
                                  std::vector<WeightMatrix::Triplet>& triplets) const {

    ASSERT(!neighbours.empty());
    triplets.assign(1, {ip, neighbours.front().payload(), 1.});
}


bool NearestNeighbour::sameAs(const DistanceWeighting& other) const {
    const auto* o = dynamic_cast<const NearestNeighbour*>(&other);
    return (o != nullptr);
}


void NearestNeighbour::json(eckit::JSON& j) const {
    j << type() << "nearest-neighbour";
}


void NearestNeighbour::print(std::ostream& out) const {
    out << "NearestNeighbour[]";
}


void NearestNeighbour::hash(eckit::MD5& h) const {
    std::ostringstream s;
    s << *this;
    h.add(s.str());
}


static const DistanceWeightingBuilder<NearestNeighbour> __distance("nearest-neighbour");


}  // namespace mir::method::knn::distance
