// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/method/gridbox/GridBoxMethod.h"


namespace mir::method::gridbox {


struct GridBoxStatistics final : GridBoxMethod {
    explicit GridBoxStatistics(const param::MIRParametrisation&);

    void assemble(util::MIRStatistics&, WeightMatrix&, const repres::Representation& in,
                  const repres::Representation& out) const override;
    const char* type() const override;
    int version() const override;
    void json(eckit::JSON&) const override;

private:
    std::string interpolationStatistics_;
};


}  // namespace mir::method::gridbox
