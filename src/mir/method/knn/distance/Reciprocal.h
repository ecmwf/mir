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


#pragma once

#include "mir/method/knn/distance/DistanceWeighting.h"


namespace mir {
namespace method {
namespace knn {
namespace distance {


/// Reciprocal: the mean of the values, weighted by the (square of the) inverse of their distance from the target point.
/// If a source point lies exactly on the target point then its value is used directly and the rest of the values
/// discarded.
struct Reciprocal : DistanceWeighting {
    Reciprocal(const param::MIRParametrisation&);
    void operator()(size_t ip, const Point3& point, const std::vector<search::PointSearch::PointValueType>& neighbours,
                    std::vector<WeightMatrix::Triplet>& triplets) const override;

private:
    double r_;
    double r2_;
    bool sameAs(const DistanceWeighting&) const override;
    void print(std::ostream&) const override;
    void hash(eckit::MD5&) const override;
};


}  // namespace distance
}  // namespace knn
}  // namespace method
}  // namespace mir
