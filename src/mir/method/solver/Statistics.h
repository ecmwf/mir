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

#include "mir/method/solver/Solver.h"


namespace mir {
namespace stats {
class Field;
}
}  // namespace mir


namespace mir {
namespace method {
namespace solver {


/// Non-linear system solving by calculating statistics on sets of input points (substitutes matrix multiply)
struct Statistics final : solver::Solver {
    Statistics(const param::MIRParametrisation&, stats::Field*);

    Statistics(const Statistics&) = delete;
    void operator=(const Statistics&) = delete;

    void solve(const MethodWeighted::Matrix& A, const MethodWeighted::WeightMatrix& W, MethodWeighted::Matrix& B,
               const double& missingValue) const override;

private:
    bool sameAs(const Solver&) const override;

    void print(std::ostream&) const override;

    void hash(eckit::MD5&) const override;

    std::unique_ptr<stats::Field> stats_;
};


}  // namespace solver
}  // namespace method
}  // namespace mir
