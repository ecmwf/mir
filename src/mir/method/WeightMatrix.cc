/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Tiago Quintino
/// @date May 2015


#include "mir/method/WeightMatrix.h"

#include <cmath>
#include "eckit/exception/Exceptions.h"
#include "eckit/linalg/LinearAlgebra.h"
#include "eckit/linalg/Vector.h"
#include "eckit/log/Plural.h"
#include "mir/config/LibMir.h"
#include "mir/util/Compare.h"


namespace mir {
namespace method {


void WeightMatrix::setFromTriplets(const std::vector<WeightMatrix::Triplet>& triplets) {
    ASSERT(rows());
    ASSERT(cols());

    SparseMatrix M(rows(), cols(), triplets);

    swap(M);
}

void WeightMatrix::print(std::ostream& out) const {
    out << "WeightMatrix["
        <<  "rows=" << rows()
        << ",cols=" << cols()
        << "]";
}

void WeightMatrix::multiply(const WeightMatrix::Vector& values, WeightMatrix::Vector& result) const {

    // TODO: linear algebra backend should depend on parametrisation
    eckit::linalg::LinearAlgebra::backend().spmv(*this, values, result);
}

void WeightMatrix::multiply(const WeightMatrix::Matrix& values, WeightMatrix::Matrix& result) const {

    eckit::Log::debug<LibMir>() << "MethodWeighted::multiply: "
                                   "A[" << rows()        << ',' << cols()        << "] * "
                                   "B[" << values.rows() << ',' << values.cols() << "] = "
                                   "C[" << result.rows() << ',' << result.cols() << "]" << std::endl;

    eckit::Log::info() << "MethodWeighted::multiply: "
                                   "A[" << rows()        << ',' << cols()        << "] * "
                                   "B[" << values.rows() << ',' << values.cols() << "] = "
                                   "C[" << result.rows() << ',' << result.cols() << "]" << std::endl;

    ASSERT(values.rows() == cols());
    ASSERT(result.rows() == rows());
    ASSERT(values.cols() == result.cols());

    // when interpolating, the general case is for single-column values/result vectors
    if (values.cols() == 1) {
        // FIXME: remove this const cast once Vector provides read-only view
        eckit::linalg::Vector vi(const_cast<double *>(values.data()), values.rows());
        eckit::linalg::Vector vo(result.data(), result.rows());

        eckit::linalg::LinearAlgebra::backend().spmv(*this, vi, vo);
    } else {
        eckit::linalg::LinearAlgebra::backend().spmm(*this, values, result);
    }
}

void WeightMatrix::cleanup(const double& pruneEpsilon) {
    size_t fixed = 0;
    size_t count = 0;

    for (Size i = 0; i < rows(); ++i) {

        double removed = 0;
        size_t non_zero = 0;

        for (iterator it = begin(i); it != end(i); ++it) {
            const double a = *it;
            if (fabs(a) < pruneEpsilon) {
                removed += a;
                *it = 0;
                fixed++;
            } else {
                non_zero++;
            }
            count++;
        }

        if (removed && non_zero) {
            double d = removed / non_zero;
            for (iterator it = begin(i); it != end(i); ++it) {
                const double a = *it;
                if (a) {
                    *it = a + d;
                }
            }
        }
    }

    if (fixed) {
        size_t r = rows();
        size_t c = cols();
        size_t total = r * c;
        eckit::Log::debug<LibMir>() << "MethodWeighted::cleanupMatrix fixed "
                                    << eckit::Plural(fixed, "value") << " out of " << eckit::BigNum(count)
                                    << " (matrix is " << eckit::BigNum(r) << "x" << eckit::BigNum(c) << ", total="
                                    << eckit::BigNum(total) << ")" << std::endl;
    }
    prune(0.0);
}

void WeightMatrix::validate(const char *when) const {

    size_t errors = 0;
    for (Size i = 0; i < rows(); i++) {

        // check for W(i,j)<0, or W(i,j)>1, or sum(W(i,:))!=(0,1)
        double sum = 0.;
        bool ok  = true;

        for (const_iterator it = begin(i); it != end(i); ++it) {
            const double &a = *it;
            ok = ok &&
                 eckit::FloatCompare<double>::isApproximatelyGreaterOrEqual(a, 0) &&
                 eckit::FloatCompare<double>::isApproximatelyGreaterOrEqual(1, a);
            sum += a;
        }

        ok = ok &&
             util::compare::is_approx_zero(sum) &&
             util::compare::is_approx_one(sum);

        // log issues, per row
        if (!ok) {
            if (errors < 50) {
                if (!errors) {
                    eckit::Log::debug<LibMir>() << "WeightMatrix::validate(" << when << ") failed " << std::endl;
                }

                eckit::Log::debug<LibMir>() << "Row: " << i;
                size_t n = 0;
                for (const_iterator it = begin(i); it != end(i); ++it, ++n) {
                    if (n > 10) {
                        eckit::Log::debug<LibMir>() << " ...";
                        break;
                    }
                    eckit::Log::debug<LibMir>() << " [" << *it << "]";
                }

                eckit::Log::debug<LibMir>() << " sum=" << sum << ", 1-sum " << (1 - sum) << std::endl;
            } else if (errors == 50) {
                eckit::Log::debug<LibMir>() << "..." << std::endl;
            }
            errors++;

        }
    }

    if (errors) {
        std::ostringstream os;
        os << "WeightMatrix::validate(" << when << ") failed ";
    }
}


}  // namespace method
}  // namespace mir
