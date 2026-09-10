// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/method/solver/Solver.h"


namespace eckit::linalg {
class LinearAlgebraSparse;
}  // namespace eckit::linalg


namespace mir::method::solver {


/// Linear system solving by matrix multiplication
struct Multiply : Solver {
    explicit Multiply(const param::MIRParametrisation&);

    void solve(const DenseMatrix& A, const WeightMatrix& W, DenseMatrix& B, const double& missingValue) const override;

private:
    bool sameAs(const Solver&) const override;
    void print(std::ostream&) const override;
    void hash(eckit::MD5&) const override;
    void json(eckit::JSON&) const override;

    const eckit::linalg::LinearAlgebraSparse& backend_;
};


}  // namespace mir::method::solver
