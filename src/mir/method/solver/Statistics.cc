// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/method/solver/Statistics.h"

#include <cmath>
#include <sstream>

#include "eckit/log/JSON.h"
#include "eckit/utils/MD5.h"

#include "mir/method/WeightMatrix.h"
#include "mir/method/solver/Solver.h"
#include "mir/stats/Field.h"
#include "mir/util/Exceptions.h"


namespace mir::method::solver {


Statistics::Statistics(const param::MIRParametrisation& param, stats::Field* fieldStats) :
    Solver(param), stats_(fieldStats) {
    ASSERT(stats_);
}


void Statistics::solve(const DenseMatrix& A, const WeightMatrix& W, DenseMatrix& B, const double& missingValue) const {
    // statistics of columns per row
    ASSERT(A.cols() == 1);
    ASSERT(B.cols() == 1);
    ASSERT(W.cols() == A.rows());
    ASSERT(W.rows() == B.rows());
    auto N = A.rows();

    WeightMatrix::const_iterator it(W);
    for (WeightMatrix::Size r = 0; r < W.rows(); ++r) {
        // comparison v == missingValue holds iff !isnan(missingValue)
        stats_->reset(missingValue, !std::isnan(missingValue));

        for (; it != W.end(r); ++it) {
            ASSERT(it.col() < N);
            stats_->count(A[it.col()]);
        }

        auto value = stats_->value();
        B(r, 0)    = static_cast<WeightMatrix::Scalar>(std::isnan(value) ? missingValue : value);
    }
}

void Statistics::print(std::ostream& out) const {
    out << "Statistics[stats=" << *stats_ << "]";
}


bool Statistics::sameAs(const Solver& /*unused*/) const {
    return false; /* data-dependant */
}


void Statistics::hash(eckit::MD5& h) const {
    std::ostringstream s;
    s << *this;
    h.add(s.str());
}


void Statistics::json(eckit::JSON& j) const {
    j.startObject();
    j << "type"
      << "statistics";
    j << "statistics" << *stats_;
    j.endObject();
}


}  // namespace mir::method::solver
