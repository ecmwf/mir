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

#include <memory>

#include "mir/method/knn/KNearestNeighbours.h"
#include "mir/method/knn/distance/NoDistanceWeighting.h"


namespace mir::method::knn {


struct KNearestStatistics final : KNearestNeighbours {
    explicit KNearestStatistics(const param::MIRParametrisation&);

private:
    const char* name() const override;
    bool sameAs(const Method&) const override;
    const pick::Pick& pick() const override;
    const distance::DistanceWeighting& distanceWeighting() const override;

    std::unique_ptr<const pick::Pick> pick_;
    distance::NoDistanceWeighting distanceWeighting_;
};


}  // namespace mir::method::knn
