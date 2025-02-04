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

#include "eckit/log/JSON.h"

#include "mir/method/solver/Solver.h"


namespace mir::stats {
class Field;
}  // namespace mir::stats


namespace mir::method::solver {


/// Non-linear system solving by calculating statistics on sets of input points (substitutes matrix multiply)
struct Statistics final : solver::Solver {
    Statistics(const param::MIRParametrisation&, stats::Field*);

    Statistics(const Statistics&) = delete;
    Statistics(Statistics&&)      = delete;

    void operator=(const Statistics&) = delete;
    void operator=(Statistics&&)      = delete;

    void solve(const DenseMatrix& A, const WeightMatrix& W, DenseMatrix& B, const double& missingValue) const override;

private:
    bool sameAs(const Solver&) const override;
    void print(std::ostream&) const override;
    void hash(eckit::MD5&) const override;
    void json(eckit::JSON&) const override;

    std::unique_ptr<stats::Field> stats_;
};


}  // namespace mir::method::solver
