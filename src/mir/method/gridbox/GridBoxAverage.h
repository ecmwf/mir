// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/method/gridbox/GridBoxMethod.h"


namespace mir::method::gridbox {


struct GridBoxAverage final : GridBoxMethod {
    explicit GridBoxAverage(const param::MIRParametrisation&);

    void assemble(util::MIRStatistics&, WeightMatrix&, const repres::Representation& in,
                  const repres::Representation& out) const override;
    const char* type() const override;
    int version() const override;
};


}  // namespace mir::method::gridbox
