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


/**
/* Calculates the mean of the values weighted (multiplied) by the following:
 * - if tolerance is not zero: exp(−distance2/tolerance2)
 * - if tolerance is zero: 1. if the point is on the target point, 0. otherwise
 * @note With a tolerance of zero, it calculates the number of input points that lie exactly on each target point.
 */
struct ExponentialMean : DistanceWeighting {
    ExponentialMean(const param::MIRParametrisation&);
    void operator()(size_t ip, const Point3& point, const std::vector<search::PointSearch::PointValueType>& neighbours,
                    std::vector<WeightMatrix::Triplet>& triplets) const override;

private:
    double tolerance_;
    double tolerance2_;
    bool sameAs(const DistanceWeighting&) const override;
    void print(std::ostream&) const override;
    void hash(eckit::MD5&) const override;
};


}  // namespace distance
}  // namespace knn
}  // namespace method
}  // namespace mir
