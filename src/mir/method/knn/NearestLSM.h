// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/method/knn/KNearestNeighbours.h"

#include "mir/method/knn/distance/DistanceWeightingWithLSM.h"


namespace mir::method::knn {


struct NearestLSM final : KNearestNeighbours {
    explicit NearestLSM(const param::MIRParametrisation&);

private:
    void assemble(util::MIRStatistics&, WeightMatrix&, const repres::Representation& in,
                  const repres::Representation& out) const override;

    /// Update matrix to account for field masked values
    void applyMasks(WeightMatrix&, const lsm::LandSeaMasks&) const override;

    lsm::LandSeaMasks getMasks(const repres::Representation& in, const repres::Representation& out) const override;

    const char* type() const override;
    bool sameAs(const Method&) const override;

    const pick::Pick& pick() const override;
    const distance::DistanceWeighting& distanceWeighting() const override;

    std::unique_ptr<const pick::Pick> pick_;
    distance::DistanceWeightingWithLSM distanceWeighting_;
};


}  // namespace mir::method::knn
