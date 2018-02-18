/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/method/knn/distance/NearestNeighbour.h"


namespace mir {
namespace method {
namespace knn {
namespace distance {


NearestNeighbour::NearestNeighbour(const param::MIRParametrisation&) {
}


void NearestNeighbour::operator()(
        size_t ip,
        const eckit::geometry::Point3&,
        const std::vector<util::PointSearch::PointValueType>& neighbours,
        std::vector<WeightMatrix::Triplet>& triplets ) const {

    ASSERT(!neighbours.empty());
    triplets.assign(1, WeightMatrix::Triplet(ip, neighbours.front().payload(), 1.));
}


static DistanceWeightingBuilder<NearestNeighbour> __distance("nearest-neighbour");


}  // namespace distance
}  // namespace knn
}  // namespace method
}  // namespace mir

