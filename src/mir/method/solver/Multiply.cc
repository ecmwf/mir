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


#include "mir/method/solver/Multiply.h"

#include <ostream>
#include <sstream>

#include "eckit/linalg/LinearAlgebra.h"
#include "eckit/linalg/Vector.h"
#include "eckit/utils/MD5.h"

#include "mir/util/Exceptions.h"


namespace mir {
namespace method {
namespace solver {


void Multiply::solve(const MethodWeighted::Matrix& A, const MethodWeighted::WeightMatrix& W, MethodWeighted::Matrix& B,
                     const double&) const {
    ASSERT(A.rows() == W.cols());
    ASSERT(B.rows() == W.rows());
    ASSERT(A.cols() == B.cols());

    // The general case is for single-column values/result vectors
    // FIXME remove const_cast once Vector provides read-only view
    if (A.cols() == 1) {
        eckit::linalg::Vector a(const_cast<double*>(A.data()), A.rows());
        eckit::linalg::Vector b(B.data(), B.rows());

        eckit::linalg::LinearAlgebra::backend().spmv(W, a, b);
    }
    else {
        eckit::linalg::LinearAlgebra::backend().spmm(W, A, B);
    }
}


bool Multiply::sameAs(const Solver& other) const {
    return (dynamic_cast<const Multiply*>(&other) != nullptr);
}


void Multiply::print(std::ostream& out) const {
    out << "Multiply[]";
}


void Multiply::hash(eckit::MD5& h) const {
    std::ostringstream s;
    s << *this;
    h.add(s.str());
}


}  // namespace solver
}  // namespace method
}  // namespace mir
