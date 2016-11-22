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


#include <cmath>
#include "eckit/exception/Exceptions.h"
#include "eckit/linalg/LinearAlgebra.h"
#include "eckit/linalg/Vector.h"
#include "eckit/log/Plural.h"
#include "atlas/interpolation/Intersect.h"
#include "mir/config/LibMir.h"
#include "mir/method/WeightMatrix.h"
#include "mir/util/Compare.h"


namespace mir {
namespace method {

WeightMatrix::WeightMatrix() : matrix_() {}

WeightMatrix::WeightMatrix(Size rows, Size cols) : matrix_(rows, cols) {}

void WeightMatrix::save(const eckit::PathName &path) const {
    matrix_.save(path);
}

void WeightMatrix::load(const eckit::PathName &path)  {
    matrix_.load(path);
}

void WeightMatrix::setIdentity()
{
    ASSERT(matrix_.rows());
    ASSERT(matrix_.cols());

    SparseMatrix M;
    M.setIdentity(matrix_.rows(), matrix_.cols());

    matrix_.swap(M);
}

void WeightMatrix::setFromTriplets(const std::vector<WeightMatrix::Triplet>& triplets)
{
    ASSERT(matrix_.rows());
    ASSERT(matrix_.cols());

    SparseMatrix M(matrix_.rows(), matrix_.cols(), triplets);

    matrix_.swap(M);
}

void WeightMatrix::prune(double value) {
    matrix_.prune(value);
}

void WeightMatrix::print(std::ostream& out) const {
    out << "WeightMatrix["
        <<  "rows=" << rows()
        << ",cols=" << cols()
        << "]";
}

void WeightMatrix::multiply(const WeightMatrix::Vector& values, WeightMatrix::Vector& result) const {

    // TODO: linear algebra backend should depend on parametrisation
    eckit::linalg::LinearAlgebra::backend().spmv(matrix_, values, result);
}

void WeightMatrix::multiply(const WeightMatrix::Matrix& values, WeightMatrix::Matrix& result) const {
    eckit::Log::debug<LibMir>() << "MethodWeighted::multiply: "
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

        eckit::linalg::LinearAlgebra::backend().spmv(matrix_, vi, vo);
    } else {
        eckit::linalg::LinearAlgebra::backend().spmm(matrix_, values, result);
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

    using eckit::linalg::Index;

    using mir::util::compare::is_approx_one;
    using mir::util::compare::is_approx_zero;

    size_t errors = 0;

    for (Size i = 0; i < rows(); i++) {

        // check for W(i,j)<0, or W(i,j)>1, or sum(W(i,:))!=(0,1)
        double sum = 0.;
        bool ok  = true;

        for (const_iterator it = begin(i); it != end(i); ++it) {
            const double &a = *it;
            if (!eckit::FloatCompare<double>::isApproximatelyGreaterOrEqual(a, 0)) {
                ok = false;
            }
            if (!eckit::FloatCompare<double>::isApproximatelyGreaterOrEqual(1, a)) {
                ok = false;
            }
            sum += a;
        }

        if (!is_approx_zero(sum) && !is_approx_one(sum)) {
            ok = false;
        }

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

size_t WeightMatrix::footprint() const {
    return matrix_.footprint();
}

}  // namespace method
}  // namespace mir
