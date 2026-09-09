// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/method/MethodWeighted.h"


namespace mir::method::gridbox {


class GridBoxMethod : public MethodWeighted {
public:
    using MethodWeighted::MethodWeighted;

private:
    void hash(eckit::MD5&) const override;
    bool sameAs(const Method&) const override;
    void print(std::ostream&) const override;
    WeightMatrix::Check validateMatrixWeights() const override;
};


}  // namespace mir::method::gridbox
