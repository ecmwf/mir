/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#ifndef mir_method_knn_distance_NearestNeighbourWithLowestIndex_h
#define mir_method_knn_distance_NearestNeighbourWithLowestIndex_h

#include "mir/method/knn/distance/DistanceWeighting.h"


namespace mir {
namespace method {
namespace knn {
namespace distance {


struct NearestNeighbourWithLowestIndex : DistanceWeighting {
    NearestNeighbourWithLowestIndex(const param::MIRParametrisation&);
    void operator()(
            size_t ip,
            const eckit::geometry::Point3& point,
            const std::vector<util::PointSearch::PointValueType>& neighbours,
            std::vector<WeightMatrix::Triplet>& triplets) const;

};


}  // namespace distance
}  // namespace knn
}  // namespace method
}  // namespace mir


#endif

