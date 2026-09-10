// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/method/knn/KNearestNeighbours.h"

#include <memory>


namespace mir::method::knn {


struct KNearest final : KNearestNeighbours {
    explicit KNearest(const param::MIRParametrisation&);

private:
    const char* type() const override;
    bool sameAs(const Method&) const override;
    const pick::Pick& pick() const override;
    const distance::DistanceWeighting& distanceWeighting() const override;

    std::unique_ptr<const pick::Pick> pick_;
    std::unique_ptr<const distance::DistanceWeighting> distanceWeighting_;
};


}  // namespace mir::method::knn
