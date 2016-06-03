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

#include "eckit/exception/Exceptions.h"
#include "eckit/log/Plural.h"

#include "eckit/linalg/LinearAlgebra.h"
#include "eckit/linalg/Vector.h"

#include "atlas/interpolation/Intersect.h"

#include "mir/method/WeightMatrix.h"
#include "mir/util/Compare.h"
#include "mir/log/MIR.h"

#include <cmath>

using eckit::linalg::Index;

using mir::util::compare::is_approx_zero;
using mir::util::compare::is_approx_one;

namespace mir {
namespace method {

WeightMatrix::WeightMatrix() : matrix_() {}

WeightMatrix::WeightMatrix(Index rows, Index cols)
    : matrix_(rows, cols) {}

void WeightMatrix::save(const eckit::PathName &path) const {
    matrix_.save(path);
}

void WeightMatrix::load(const eckit::PathName &path)  {
    matrix_.load(path);
}

void WeightMatrix::setFromTriplets(const std::vector<WeightMatrix::Triplet>& triplets) {
    matrix_.setFromTriplets(triplets);
}

void WeightMatrix::setIdentity() {
    matrix_.setIdentity();
}

void WeightMatrix::prune(double value) {
    matrix_.prune(value);
}

void WeightMatrix::print(std::ostream& out) const {
    out << "WeightMatrix[rows="
        << rows()
        << ",cols="
        << cols()
        << "]";
}

void WeightMatrix::multiply(const std::vector<double>& values, std::vector<double>& result) const {

    // FIXME: remove this const cast once Vector provides read-only view
    eckit::linalg::Vector vi(const_cast<double *>(values.data()), values.size());
    eckit::linalg::Vector vo(result.data(), result.size());

    // TODO: linear algebra backend should depend on parametrisation
    eckit::linalg::LinearAlgebra::backend().spmv(matrix_, vi, vo);
}

void WeightMatrix::cleanup() {
    size_t fixed = 0;
    size_t count = 0;
    for (Index i = 0; i < rows(); i++) {
        double removed = 0;
        size_t non_zero = 0;

        for (WeightMatrix::inner_iterator j(*this, i); j; ++j) {
            const double a = *j;
            if (fabs(a) < atlas::interpolation::parametricEpsilon) {
                removed += a;
                *j = 0;
                fixed++;
            } else {
                non_zero++;
            }
            count++;
        }

        if (removed && non_zero) {
            double d = removed / non_zero;
            for (WeightMatrix::inner_iterator j(*this, i); j; ++j) {
                const double a = *j;
                if (a) {
                    *j = a + d;
                }
            }
        }
    }

    if (fixed) {
        size_t r = rows();
        size_t c = cols();
        size_t total = r * c;
        eckit::Log::trace<MIR>() << "MethodWeighted::cleanupMatrix fixed "
                                 << eckit::Plural(fixed, "value") << " out of " << eckit::BigNum(count)
                                 << " (matrix is " << eckit::BigNum(r) << "x" << eckit::BigNum(c) << ", total=" <<
                                 eckit::BigNum(total) << ")" << std::endl;
    }
    prune(0.0);
}

void WeightMatrix::validate(const char *when) const {

    size_t errors = 0;

    for (Index i = 0; i < rows(); i++) {

        // check for W(i,j)<0, or W(i,j)>1, or sum(W(i,:))!=(0,1)
        double sum = 0.;
        bool ok  = true;

        for (WeightMatrix::inner_const_iterator j(*this, i); j; ++j) {
            const double &a = *j;
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
                    eckit::Log::trace<MIR>() << "checkMatrixWeights(" << when << ") failed " << std::endl;
                }

                eckit::Log::trace<MIR>() << "Row: " << i;
                size_t n = 0;
                for (WeightMatrix::inner_const_iterator j(*this, i); j; ++j, ++n) {
                    if (n > 10) {
                        eckit::Log::trace<MIR>() << " ...";
                        break;
                    }
                    eckit::Log::trace<MIR>() << " [" << *j << "]";
                }

                eckit::Log::trace<MIR>() << " sum=" << sum << ", 1-sum " << (1 - sum) << std::endl;
            } else if (errors == 50) {
                eckit::Log::trace<MIR>() << "..." << std::endl;
            }
            errors++;

        }
    }

    if (errors) {
        std::ostringstream os;
        os << "checkMatrixWeights(" << when << ") failed ";
    }
}

}  // namespace method
}  // namespace mir

