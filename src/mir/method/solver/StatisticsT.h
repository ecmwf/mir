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


#include <sstream>

#include "eckit/utils/MD5.h"

#include "mir/method/WeightMatrix.h"
#include "mir/method/solver/Solver.h"
#include "mir/util/Exceptions.h"


namespace mir {
namespace method {
namespace solver {


/// Non-linear system solving by calculating statistics on sets of input points (substitutes matrix multiply)
template <typename STATS>
struct StatisticsT final : solver::Solver {
    StatisticsT(const param::MIRParametrisation& param) : Solver(param), stats_(param) {}

    StatisticsT(const StatisticsT&) = delete;
    void operator=(const StatisticsT&) = delete;

    void solve(const MethodWeighted::Matrix& A, const MethodWeighted::WeightMatrix& W, MethodWeighted::Matrix& B,
               const double& missingValue) const override {

        // statistics of columns per row
        ASSERT(A.cols() == 1);
        ASSERT(B.cols() == 1);
        ASSERT(W.cols() == A.rows());
        ASSERT(W.rows() == B.rows());
        auto N = A.rows();

        WeightMatrix::const_iterator it(W);
        for (WeightMatrix::Size r = 0; r < W.rows(); ++r) {
            stats_.reset(missingValue, missingValue == missingValue);

            for (; it != W.end(r); ++it) {
                ASSERT(it.col() < N);
                stats_.count(A[it.col()]);
            }

            B(r, 0) = static_cast<WeightMatrix::Scalar>(stats_.value());
        }
    }

private:
    bool sameAs(const Solver&) const override { return false; /* data-dependant */ }

    void print(std::ostream& out) const override { out << "StatisticsT[stats=" << stats_.name() << "]"; }

    void hash(eckit::MD5& h) const override {
        std::ostringstream s;
        s << *this;
        h.add(s.str());
    }

    mutable STATS stats_;
};


}  // namespace solver
}  // namespace method
}  // namespace mir
