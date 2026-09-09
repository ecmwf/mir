// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/method/knn/distance/DistanceWeightingWithLSM.h"


namespace mir::lsm {
class LandSeaMasks;
}  // namespace mir::lsm


namespace mir::method::knn::distance {


// Note: DistanceWeightingFactory cannot instantiate this because it
// requires the LandSeaMasks from the interpolation method
struct NearestLSMWithLowestIndex : DistanceWeightingWithLSM {
    NearestLSMWithLowestIndex(const param::MIRParametrisation&, const lsm::LandSeaMasks&);
    void operator()(size_t ip, const Point3& point, const std::vector<search::PointSearch::PointValueType>& neighbours,
                    std::vector<WeightMatrix::Triplet>& triplets) const override;

private:
    bool sameAs(const DistanceWeighting&) const override;
    void json(eckit::JSON&) const override;
    void print(std::ostream&) const override;
    void hash(eckit::MD5&) const override;

    const std::vector<bool>& imask_;
    const std::vector<bool>& omask_;
};


}  // namespace mir::method::knn::distance
