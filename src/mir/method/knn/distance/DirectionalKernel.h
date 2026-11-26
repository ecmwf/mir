// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/method/knn/distance/DistanceWeighting.h"
#include "mir/method/knn/distance/Kernel.h"


namespace mir::method::knn::distance {


/**
 * @brief Directional (elliptical) smoothing kernel, on the local tangent plane, with weights adding up to one
 *
 * w_j = K(s_j) / sum_k K(s_k), s = sqrt((u / a)^2 + (v / b)^2), where (u, v) is the offset along and across the
 * direction "kernel-angle" (degrees, counter-clockwise from east: 0 is east-west, 90 is north-south), with radii a =
 * "kernel-radius" (along, default "distance") and b = "kernel-radius-across" (default a). The profile K is "kernel"
 * (default gaussian; see KernelProfile). If no neighbour has a positive weight, the nearest one is used.
 */
class DirectionalKernel final : public DistanceWeighting {
public:
    explicit DirectionalKernel(const param::MIRParametrisation&);

    void operator()(size_t ip, const Point3& point, const std::vector<search::PointSearch::PointValueType>& neighbours,
                    std::vector<WeightMatrix::Triplet>& triplets) const override;

private:
    bool sameAs(const DistanceWeighting&) const override;
    void json(eckit::JSON&) const override;
    void print(std::ostream&) const override;
    void hash(eckit::MD5&) const override;

    KernelProfile profile_;
    double along_;
    double across_;
    double angle_;
    double cos_;
    double sin_;
};


}  // namespace mir::method::knn::distance
