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


#include "mir/method/voronoi/StatisticsT.h"

#include <ostream>
#include <sstream>

#include "eckit/utils/MD5.h"

#include "mir/method/WeightMatrix.h"
#include "mir/method/solver/Solver.h"
#include "mir/stats/detail/CentralMomentsT.h"
#include "mir/stats/detail/Counter.h"
#include "mir/util/Exceptions.h"


namespace mir {
namespace method {
namespace voronoi {


/// Non-linear system solving (substitutes matrix multiply)
template <typename STATS>
struct SolveStatisticsT final : solver::Solver {
    SolveStatisticsT(const param::MIRParametrisation& param) : Solver(param), stats_(param) {}

    SolveStatisticsT(const SolveStatisticsT&) = delete;
    void operator=(const SolveStatisticsT&) = delete;

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
    bool sameAs(const Solver& other) const override {
        auto o = dynamic_cast<const SolveStatisticsT*>(&other);
        return (o != nullptr);
    }

    void print(std::ostream& out) const override { out << "SolveStatisticsT[]"; }

    void hash(eckit::MD5& h) const override {
        std::ostringstream s;
        s << *this;
        h.add(s.str());
    }

    mutable STATS stats_;
};


/// Simple statistics on values (min, max, etc.)
template <typename T>
struct CounterStatsT : stats::detail::Counter {
    using Counter::Counter;
    double value() const { NOTIMP; /*ensure specialization*/ }
};


struct Maximum {};
struct Minimum {};
struct CountAboveUpperLimit {};
struct CountBelowLowerLimit {};
struct Count {};


template <>
double CounterStatsT<Maximum>::value() const {
    return max();
}


template <>
double CounterStatsT<Minimum>::value() const {
    return min();
}


template <>
double CounterStatsT<CountAboveUpperLimit>::value() const {
    return double(countAboveUpperLimit());
}


template <>
double CounterStatsT<CountBelowLowerLimit>::value() const {
    return double(countBelowLowerLimit());
}


template <>
double CounterStatsT<Count>::value() const {
    return double(count() - missing());
}


/// Central moment statistics on values (mean, stddev, etc.)
template <typename STATS>
struct StatsT : stats::detail::Counter, STATS {
    using Counter::Counter;
    virtual double value() const = 0;
    void count(const double& value) {
        if (Counter::count(value)) {
            STATS::operator()(value);
        }
    }
    void reset(double missingValue, bool hasMissing) {
        Counter::reset(missingValue, hasMissing);
        STATS::reset();
    }
};


struct Mean : StatsT<stats::detail::CentralMomentsT<double>> {
    using StatsT::StatsT;
    double value() const override { return mean(); }
};


struct Variance : StatsT<stats::detail::CentralMomentsT<double>> {
    using StatsT::StatsT;
    double value() const override { return variance(); }
};


struct Skewness : StatsT<stats::detail::CentralMomentsT<double>> {
    using StatsT::StatsT;
    double value() const override { return skewness(); }
};


struct Kurtosis : StatsT<stats::detail::CentralMomentsT<double>> {
    using StatsT::StatsT;
    double value() const override { return kurtosis(); }
};


struct StandardDeviation : StatsT<stats::detail::CentralMomentsT<double>> {
    using StatsT::StatsT;
    double value() const override { return standardDeviation(); }
};


template <typename T>
StatisticsT<T>::StatisticsT(const param::MIRParametrisation& param) : VoronoiMethod(param) {
    setSolver(new SolveStatisticsT<T>(param));
}


template <>
const char* StatisticsT<CounterStatsT<Maximum>>::name() const {
    return "voronoi-maximum";
}


template <>
const char* StatisticsT<CounterStatsT<Minimum>>::name() const {
    return "voronoi-minimum";
}


template <>
const char* StatisticsT<CounterStatsT<CountAboveUpperLimit>>::name() const {
    return "voronoi-count-above-upper-limit";
}


template <>
const char* StatisticsT<CounterStatsT<CountBelowLowerLimit>>::name() const {
    return "voronoi-count-below-lower-limit";
}


template <>
const char* StatisticsT<CounterStatsT<Count>>::name() const {
    return "voronoi-count";
}


template <>
const char* StatisticsT<Mean>::name() const {
    return "voronoi-mean";
}


template <>
const char* StatisticsT<Variance>::name() const {
    return "voronoi-variance";
}


template <>
const char* StatisticsT<Skewness>::name() const {
    return "voronoi-skewness";
}


template <>
const char* StatisticsT<Kurtosis>::name() const {
    return "voronoi-kurtosis";
}


template <>
const char* StatisticsT<StandardDeviation>::name() const {
    return "voronoi-stddev";
}


static MethodBuilder<StatisticsT<CounterStatsT<Maximum>>> __builder_1("voronoi-maximum");
static MethodBuilder<StatisticsT<CounterStatsT<Minimum>>> __builder_2("voronoi-minimum");
static MethodBuilder<StatisticsT<CounterStatsT<CountAboveUpperLimit>>> __builder_3("voronoi-count-above-upper-limit");
static MethodBuilder<StatisticsT<CounterStatsT<CountBelowLowerLimit>>> __builder_4("voronoi-count-below-lower-limit");
static MethodBuilder<StatisticsT<CounterStatsT<Count>>> __builder_5("voronoi-count");

static MethodBuilder<StatisticsT<Mean>> __builder_6("voronoi-mean");
static MethodBuilder<StatisticsT<Variance>> __builder_7("voronoi-variance");
static MethodBuilder<StatisticsT<Skewness>> __builder_8("voronoi-skewness");
static MethodBuilder<StatisticsT<Kurtosis>> __builder_9("voronoi-kurtosis");
static MethodBuilder<StatisticsT<StandardDeviation>> __builder_10("voronoi-stddev");


}  // namespace voronoi
}  // namespace method
}  // namespace mir
