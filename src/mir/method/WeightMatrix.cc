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

#include "atlas/geometry/Intersect.h"
// #include "eckit/filesystem/PathName.h"
// #include "eckit/io/BufferedHandle.h"
#include "eckit/log/Plural.h"
#include "mir/util/Compare.h"

#include <cmath>

using mir::util::compare::is_approx_zero;
using mir::util::compare::is_approx_one;

namespace mir {
namespace method {

void WeightMatrix::save(const eckit::PathName &path) const {
    matrix_.save(path);
}

void WeightMatrix::load(const eckit::PathName &path)  {
    matrix_.load(path);
}

void WeightMatrix::cleanup() {
    size_t fixed = 0;
    size_t count = 0;
    for (size_t i = 0; i < rows(); i++) {
        double removed = 0;
        size_t non_zero = 0;

        for (WeightMatrix::inner_iterator j(*this, i); j; ++j) {
            const double a = *j;
            if (fabs(a) < atlas::geometry::parametricEpsilon) {
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
        eckit::Log::info() << "MethodWeighted::cleanupMatrix fixed "
                           << eckit::Plural(fixed, "value") << " out of " << eckit::BigNum(count)
                           << " (matrix is " << eckit::BigNum(rows()) << "x" << eckit::BigNum(cols()) << ", total=" <<
                           eckit::BigNum(rows() * cols()) << ")" << std::endl;
    }
    prune(0.0);
}

void WeightMatrix::validate(const char *when) const {

    size_t errors = 0;

    for (size_t i = 0; i < rows(); i++) {

        // check for W(i,j)<0, or W(i,j)>1, or sum(W(i,:))!=(0,1)
        double sum = 0.;
        bool ok  = true;

        for (WeightMatrix::inner_const_iterator j(*this, i); j; ++j) {
            const double &a = *j;
            if (!eckit::FloatCompare<double>::isGreaterApproxEqual(a, 0)) {
                ok = false;
            }
            if (!eckit::FloatCompare<double>::isGreaterApproxEqual(1, a)) {
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
                    eckit::Log::info() << "checkMatrixWeights(" << when << ") failed " << std::endl;
                }

                eckit::Log::info() << "Row: " << i;
                size_t n = 0;
                for (WeightMatrix::inner_const_iterator j(*this, i); j; ++j, ++n) {
                    if (n > 10) {
                        eckit::Log::info() << " ...";
                        break;
                    }
                    eckit::Log::info() << " [" << *j << "]";
                }

                eckit::Log::info() << " sum=" << sum << ", 1-sum " << (1 - sum) << std::endl;
            } else if (errors == 50) {
                eckit::Log::info() << "..." << std::endl;
            }
            errors++;

        }
    }

    if (errors) {
        eckit::StrStream os;
        os << "checkMatrixWeights(" << when << ") failed ";
    }
}

}  // namespace method
}  // namespace mir

