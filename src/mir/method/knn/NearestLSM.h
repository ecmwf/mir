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

    const char* name() const override;
    bool sameAs(const Method&) const override;

    const pick::Pick& pick() const override;
    const distance::DistanceWeighting& distanceWeighting() const override;

    std::unique_ptr<const pick::Pick> pick_;
    distance::DistanceWeightingWithLSM distanceWeighting_;
};


}  // namespace mir::method::knn
