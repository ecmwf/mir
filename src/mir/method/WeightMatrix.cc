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
#include <sstream>
#include <unordered_set>

#include "eckit/types/FloatCompare.h"

#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/Types.h"


namespace mir::method {


WeightMatrix::WeightMatrix(SparseMatrix::Allocator* alloc) : SparseMatrix(alloc) {}


WeightMatrix::WeightMatrix(const eckit::PathName& path) {
    load(path);
}


WeightMatrix::WeightMatrix(WeightMatrix::Size rows, WeightMatrix::Size cols) : SparseMatrix(rows, cols) {}


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
            if (std::abs(a) < pruneEpsilon) {
                if (0. < std::abs(a)) {
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
    constexpr size_t Nerrors = 10;
    size_t errors            = 0;

    std::ostringstream what;
    const char* sep = "";

    // check for weights out of bounds 0 <= W(i,j) <= 1, sum(W(i,:))=(0,1), and dulplicate column indices
    for (Size r = 0; r < rows(); r++) {
        Scalar sum = 0.;
        std::unordered_set<Size> cols;

        bool check_bounds        = true;
        bool check_no_duplicates = true;
        for (auto it = begin(r); it != end(r); ++it) {
            auto a = *it;
            check_bounds &= eckit::types::is_approximately_greater_or_equal(a, 0.) &&
                            eckit::types::is_approximately_greater_or_equal(1., a);
            sum += a;

            check_no_duplicates &= cols.insert(it.col()).second;
        }

        auto check_sum = eckit::types::is_approximately_equal(sum, 0.) || eckit::types::is_approximately_equal(sum, 1.);

        if (!check_bounds || !check_sum || !check_no_duplicates) {
            if (errors < Nerrors) {
                what << sep << "row " << r << ": ";
                const char* s = "";

                if (!check_bounds) {
                    what << s << "weights out-of-bounds";
                    s = ", ";
                }

                if (!check_sum) {
                    what << s << "weights sum not 0 or 1 (sum=" << sum << ", 1-sum=" << (1 - sum) << ")";
                    s = ", ";
                }

                if (!check_no_duplicates) {
                    what << s << "duplicate indices";
                    s = ", ";
                }

                what << s << "contents: ";
                s = "";
                for (auto it = begin(r); it != end(r); ++it) {
                    what << s << '(' << it.row() << ',' << it.col() << ',' << *it << ')';
                    s = ", ";
                }
            }

            errors++;
            sep = ", ";
        }
    }

    if (errors > 0) {
        std::ostringstream errors_str;
        errors_str << Log::Pretty{errors, {"row error"}};
        throw exception::InvalidWeightMatrix{when, errors_str.str() + ", " + what.str()};
    }
}


}  // namespace mir::method
