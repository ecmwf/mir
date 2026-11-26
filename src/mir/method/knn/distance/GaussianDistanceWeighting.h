// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/method/knn/distance/RadialKernel.h"


namespace mir::method::knn::distance {


struct GaussianDistanceWeighting : RadialKernel {
    explicit GaussianDistanceWeighting(const param::MIRParametrisation&);

private:
    bool sameAs(const DistanceWeighting&) const override;
    void json(eckit::JSON&) const override;
    void print(std::ostream&) const override;
};


}  // namespace mir::method::knn::distance
