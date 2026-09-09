// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <memory>

#include "eckit/log/JSON.h"

#include "mir/method/solver/Solver.h"


namespace mir::stats {
class Field;
}  // namespace mir::stats


namespace mir::method::solver {


/// Non-linear system solving by calculating statistics on sets of input points (substitutes matrix multiply)
struct Statistics final : solver::Solver {
    Statistics(const param::MIRParametrisation&, stats::Field*);

    void solve(const DenseMatrix& A, const WeightMatrix& W, DenseMatrix& B, const double& missingValue) const override;

private:
    bool sameAs(const Solver&) const override;
    void print(std::ostream&) const override;
    void hash(eckit::MD5&) const override;
    void json(eckit::JSON&) const override;

    std::unique_ptr<stats::Field> stats_;
};


}  // namespace mir::method::solver
