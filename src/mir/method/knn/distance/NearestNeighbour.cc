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
    j.startObject();
    j << "type"
      << "nearest-neighbour";
    j.endObject();
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
