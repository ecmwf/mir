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


#include "mir/method/WeightMatrix.h"

#include <cmath>

#include "eckit/types/FloatCompare.h"

#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/Types.h"


namespace mir {
namespace method {


WeightMatrix::WeightMatrix(const eckit::PathName& path) {
    load(path);
}


void WeightMatrix::setFromTriplets(const std::vector<WeightMatrix::Triplet>& triplets) {
    ASSERT(rows());
    ASSERT(cols());

    SparseMatrix M(rows(), cols(), triplets);

    swap(M);
}


void WeightMatrix::print(std::ostream& os) const {
    os << "WeightMatrix[";
    SparseMatrix::print(os);
    os << "]";
}


void WeightMatrix::cleanup(const double& pruneEpsilon) {
    size_t fixed = 0;
    size_t count = 0;

    for (Size i = 0; i < rows(); ++i) {

        double removed  = 0;
        size_t non_zero = 0;

        for (iterator it = begin(i); it != end(i); ++it) {
            double a = *it;
            if (std::fabs(a) < pruneEpsilon) {
                if (std::fabs(a) > 0) {
                    removed += a;
                    *it = 0;
                    fixed++;
                }
            }
            else {
                non_zero++;
            }
            count++;
        }

        if ((removed != 0.) && (non_zero > 0)) {
            double d = removed / double(non_zero);
            for (iterator it = begin(i); it != end(i); ++it) {
                double a = *it;
                if (a != 0.) {
                    *it = a + d;
                }
            }
        }
    }

    if (fixed > 0) {
        size_t r     = rows();
        size_t c     = cols();
        size_t total = r * c;
        Log::debug() << "WeightMatrix::cleanup fixed " << Log::Pretty(fixed, {"value"}) << " out of "
                     << Log::Pretty(count) << " (matrix is " << Log::Pretty(r) << "x" << Log::Pretty(c)
                     << ", total=" << Log::Pretty(total) << ")" << std::endl;
    }

    prune(0.);
}


void WeightMatrix::validate(const char* when) const {
    constexpr size_t Nerrors = 50;
    constexpr size_t Nvalues = 10;

    if (!validateMatrixEntryBounds_ && !validateMatrixRowSum_) {
        return;
    }

    size_t errors = 0;

    for (Size r = 0; r < rows(); r++) {

        // check for W(i,j)<0, or W(i,j)>1, or sum(W(i,:))!=(0,1)
        double sum = 0.;
        bool ok    = true;

        for (const_iterator it = begin(r); it != end(r); ++it) {
            double a = *it;
            if (validateMatrixEntryBounds_) {
                ok &= eckit::types::is_approximately_greater_or_equal(a, 0.) &&
                      eckit::types::is_approximately_greater_or_equal(1., a);
            }
            sum += a;
        }

        if (validateMatrixRowSum_) {
            ok &= (eckit::types::is_approximately_equal(sum, 0.) || eckit::types::is_approximately_equal(sum, 1.));
        }

        // log issues, per row
        if (!ok && Log::debug()) {
            if (errors < Nerrors) {
                if (errors == 0) {
                    Log::debug() << "WeightMatrix: validation failed (" << when << ")" << std::endl;
                }
                Log::debug() << "Row: " << r;
                size_t n = 0;
                for (const_iterator it = begin(r); it != end(r); ++it, ++n) {
                    if (n > Nvalues) {
                        Log::debug() << " ...";
                        break;
                    }
                    Log::debug() << " [" << *it << "]";
                }

                Log::debug() << " sum=" << sum << ", 1-sum " << (1 - sum) << std::endl;
            }
            else if (errors == Nerrors) {
                Log::debug() << "..." << std::endl;
            }
            errors++;
        }
    }

    if (errors > 0) {
        Log::warning() << "WeightMatrix: invalid " << Log::Pretty(errors) << " out of "
                       << Log::Pretty(rows(), {"matrix row"}) << std::endl;
    }
}


void WeightMatrix::validateMatrixEntryBounds(bool yes) {
    validateMatrixEntryBounds_ = yes;
}


void WeightMatrix::validateMatrixRowSum(bool yes) {
    validateMatrixRowSum_ = yes;
}


}  // namespace method
}  // namespace mir
