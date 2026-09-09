// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/method/MethodWeighted.h"


namespace mir::method {


class HEALPixMethod : public MethodWeighted {
public:
    using MethodWeighted::MethodWeighted;

protected:
    void hash(eckit::MD5&) const override;
    void assemble(util::MIRStatistics&, WeightMatrix&, const repres::Representation& in,
                  const repres::Representation& out) const override;
    bool sameAs(const Method&) const override;
    WeightMatrix::Check validateMatrixWeights() const override;
    int version() const override;
};


}  // namespace mir::method
