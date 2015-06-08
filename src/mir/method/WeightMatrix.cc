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
#include "eckit/filesystem/PathName.h"
#include "eckit/io/BufferedHandle.h"
#include "eckit/log/Plural.h"
#include "mir/util/Compare.h"

using mir::util::compare::is_approx_zero;
using mir::util::compare::is_approx_one;

namespace mir {
namespace method {

void WeightMatrix::save(const eckit::PathName &path) const {
    eckit::BufferedHandle f(path.fileHandle());

    f.openForWrite(0);
    eckit::AutoClose closer(f);

    // write nominal size of matrix

    Index iSize = innerSize();
    Index oSize = outerSize();

    f.write(&iSize, sizeof(iSize));
    f.write(&oSize, sizeof(oSize));

    // find all the non-zero values (aka triplets)

    std::vector<method::WeightMatrix::Triplet > trips;
    for (size_t i = 0; i < outerSize(); ++i) {
        for (method::WeightMatrix::inner_const_iterator it(*this, i); it; ++it) {
            trips.push_back(method::WeightMatrix::Triplet(it.row(), it.col(), *it));
        }
    }

    // save the number of triplets

    Index ntrips = trips.size();
    f.write(&ntrips, sizeof(ntrips));

    // now save the triplets themselves

    for (size_t i = 0; i < trips.size(); i++) {

        method::WeightMatrix::Triplet &rt = trips[i];

        Index x = rt.row();
        Index y = rt.col();
        double w = rt.value();

        f.write(&x, sizeof(x));
        f.write(&y, sizeof(y));
        f.write(&w, sizeof(w));
    }
}

void WeightMatrix::load(const eckit::PathName &path)  {

    eckit::BufferedHandle f(path.fileHandle());

    f.openForRead();
    eckit::AutoClose closer(f);

    // read inpts, outpts sizes of matrix

    Index inner, outer;

    f.read(&inner, sizeof(inner));
    f.read(&outer, sizeof(outer));

    Index npts;
    f.read(&npts, sizeof(npts));

    // read total sparse points of matrix (so we can reserve)

    std::vector<method::WeightMatrix::Triplet > insertions;

    eckit::Log::info() << "Inner: " << eckit::BigNum(inner)
                       << ", outer: " << eckit::BigNum(outer)
                       << ", number of points: " << eckit::BigNum(npts) << std::endl;

    insertions.reserve(npts);

    // read the values

    for (size_t i = 0; i < npts; i++) {
        Index x, y;
        double w;
        f.read(&x, sizeof(x));
        f.read(&y, sizeof(y));
        f.read(&w, sizeof(w));
        insertions.push_back(method::WeightMatrix::Triplet(x, y, w));
    }

    // check matrix is correctly sized
    // note that Weigths::Matrix is row-major, so rows are outer size

    ASSERT(rows() == outer);
    ASSERT(cols() == inner);

    // set the weights from the triplets

    setFromTriplets(insertions.begin(), insertions.end());
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

    using util::compare::is_approx_greater_equal;

    size_t errors = 0;

    for (size_t i = 0; i < rows(); i++) {

        // check for W(i,j)<0, or W(i,j)>1, or sum(W(i,:))!=(0,1)
        double sum = 0.;
        bool ok  = true;

        for (WeightMatrix::inner_const_iterator j(*this, i); j; ++j) {
            const double &a = *j;
            if (!is_approx_greater_equal<double>(a, 0)) {
                ok = false;
            }
            if (!is_approx_greater_equal<double>(1, a)) {
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

