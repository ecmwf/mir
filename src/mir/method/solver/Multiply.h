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
