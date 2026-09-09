// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/method/nonlinear/NonLinear.h"


namespace mir::method::nonlinear {


struct Heaviest : NonLinear {
    explicit Heaviest(const param::MIRParametrisation&);

private:
    bool treatment(DenseMatrix& A, WeightMatrix& W, DenseMatrix& B, const MIRValuesVector&,
                   const double& missingValue) const override;
    bool sameAs(const NonLinear&) const override;
    void print(std::ostream&) const override;
    void hash(eckit::MD5&) const override;

    bool modifiesMatrix(bool) const override { return true; }

    const std::string& name() const override;
};


}  // namespace mir::method::nonlinear
