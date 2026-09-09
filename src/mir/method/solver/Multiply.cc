// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/method/solver/Multiply.h"

#include <ostream>
#include <sstream>

#include "eckit/linalg/LinearAlgebraSparse.h"
#include "eckit/linalg/Vector.h"
#include "eckit/log/JSON.h"
#include "eckit/utils/MD5.h"

#include "mir/util/Exceptions.h"


namespace mir::method::solver {


Multiply::Multiply(const param::MIRParametrisation& param) :
    Solver(param), backend_(eckit::linalg::LinearAlgebraSparse::backend()) {}


void Multiply::solve(const DenseMatrix& A, const WeightMatrix& W, DenseMatrix& B,
                     const double& /*missingValue*/) const {
    ASSERT(A.rows() == W.cols());
    ASSERT(B.rows() == W.rows());
    ASSERT(A.cols() == B.cols());

    // The general case is for single-column values/result vectors
    // FIXME remove const_cast once Vector provides read-only view
    if (A.cols() == 1) {
        eckit::linalg::Vector a(const_cast<double*>(A.data()), A.rows());
        eckit::linalg::Vector b(B.data(), B.rows());

        backend_.spmv(W, a, b);
    }
    else {
        backend_.spmm(W, A, B);
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


void Multiply::json(eckit::JSON& j) const {
    j.startObject();
    j << "type"
      << "multiply";
    j.endObject();
}


}  // namespace mir::method::solver
