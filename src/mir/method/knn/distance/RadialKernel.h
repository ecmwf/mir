// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <string>

#include "mir/method/knn/distance/DistanceWeighting.h"
#include "mir/method/knn/distance/Kernel.h"


namespace mir::method::knn::distance {


/**
 * @brief Radial smoothing kernel (a convolution, with weights adding up to one): w_j = K(r_j / h) / sum_k K(r_k / h)
 *
 * The profile K is the distance weighting name (top-hat, hann, epanechnikov; see KernelProfile), h is
 * "kernel-radius" (default "distance"). If no neighbour has a positive weight, the nearest one is used.
 */
class RadialKernel : public DistanceWeighting {
public:
    RadialKernel(const param::MIRParametrisation&, const std::string& profile);

    void operator()(size_t ip, const Point3& point, const std::vector<search::PointSearch::PointValueType>& neighbours,
                    std::vector<WeightMatrix::Triplet>& triplets) const override;

protected:
    RadialKernel(const param::MIRParametrisation&, const std::string& profile, double radius);

    const KernelProfile& profile() const { return profile_; }
    double radius() const { return radius_; }

    bool sameAs(const DistanceWeighting&) const override;
    void json(eckit::JSON&) const override;
    void print(std::ostream&) const override;
    void hash(eckit::MD5&) const override;

private:
    KernelProfile profile_;
    double radius_;
};


}  // namespace mir::method::knn::distance
