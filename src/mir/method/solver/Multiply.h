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


#ifndef mir_method_solver_Multiply_h
#define mir_method_solver_Multiply_h

#include "mir/method/solver/Solver.h"


namespace mir {
namespace method {
namespace solver {


struct Multiply : Solver {
    using Solver::Solver;
    void solve(const MethodWeighted::Matrix& A, const MethodWeighted::WeightMatrix& W, MethodWeighted::Matrix& B,
               const double& missingValue) const override;

private:
    bool sameAs(const Solver&) const override;
    void print(std::ostream&) const override;
    void hash(eckit::MD5&) const override;
};


}  // namespace solver
}  // namespace method
}  // namespace mir


#endif
