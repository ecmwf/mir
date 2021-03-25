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

#include <algorithm>
#include <cmath>
#include <map>
#include <ostream>
#include <set>
#include <sstream>
#include <utility>

#include "eckit/utils/MD5.h"

#include "mir/method/WeightMatrix.h"
#include "mir/method/solver/Solver.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/stats/detail/CentralMomentsT.h"
#include "mir/stats/detail/Counter.h"
#include "mir/util/Exceptions.h"

#include "mir/util/Log.h"


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
    bool sameAs(const Solver&) const override { return false; /* data-dependant */ }

    void print(std::ostream& out) const override { out << "SolveStatisticsT[name=" << stats_.name() << "]"; }

    void hash(eckit::MD5& h) const override {
        std::ostringstream s;
        s << *this;
        h.add(s.str());
    }

    mutable STATS stats_;
};


/// Simple statistics on values (min, max, etc.)
struct CounterStats : stats::detail::Counter {
    using Counter::Counter;
    ~CounterStats() override = default;

    virtual double value() const     = 0;
    virtual const char* name() const = 0;
};


struct Maximum final : CounterStats {
    using CounterStats::CounterStats;
    double value() const override { return max(); }
    const char* name() const override { return "maximum"; }
};


struct Minimum final : CounterStats {
    using CounterStats::CounterStats;
    double value() const override { return min(); }
    const char* name() const override { return "minimum"; }
};


struct CountAboveUpperLimit final : CounterStats {
    using CounterStats::CounterStats;
    double value() const override { return double(countAboveUpperLimit()); }
    const char* name() const override { return "count-above-upper-limit"; }
};


struct CountBelowLowerLimit final : CounterStats {
    using CounterStats::CounterStats;
    double value() const override { return double(countBelowLowerLimit()); }
    const char* name() const override { return "count-below-lower-limit"; }
};


struct Count final : CounterStats {
    using CounterStats::CounterStats;
    double value() const override { return double(count() - missing()); }
    const char* name() const override { return "count"; }
};


/// Central moment statistics on values (mean, stddev, etc.)
template <typename STATS>
struct StatsT : stats::detail::Counter, STATS {
    using Counter::Counter;
    ~StatsT() override = default;

    virtual double value() const     = 0;
    virtual const char* name() const = 0;

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


struct Mean final : StatsT<stats::detail::CentralMomentsT<double>> {
    using StatsT::StatsT;
    double value() const override { return mean(); }
    const char* name() const override { return "mean"; }
};


struct Variance final : StatsT<stats::detail::CentralMomentsT<double>> {
    using StatsT::StatsT;
    double value() const override { return variance(); }
    const char* name() const override { return "variance"; }
};


struct Skewness final : StatsT<stats::detail::CentralMomentsT<double>> {
    using StatsT::StatsT;
    double value() const override { return skewness(); }
    const char* name() const override { return "skewness"; }
};


struct Kurtosis final : StatsT<stats::detail::CentralMomentsT<double>> {
    using StatsT::StatsT;
    double value() const override { return kurtosis(); }
    const char* name() const override { return "kurtosis"; }
};


struct StandardDeviation final : StatsT<stats::detail::CentralMomentsT<double>> {
    using StatsT::StatsT;
    double value() const override { return standardDeviation(); }
    const char* name() const override { return "stddev"; }
};


/// Mode statistics
template <typename T>
struct Mode : stats::detail::Counter {
    Mode(const param::MIRParametrisation& param) : Counter(param) {
        param.get("mode-disambiguate-max", disambiguateMax_);
    }

    virtual const char* name() const        = 0;
    virtual double value() const            = 0;
    virtual void count(const double& value) = 0;

    void reset(double missingValue, bool hasMissing) {
        Counter::reset(missingValue, hasMissing);
        binCount_.clear();
    }

    T mode() const {
        ASSERT(!binCount_.empty());
        auto i = binCount_.begin();
        auto j = i++;
        for (; i != binCount_.end(); ++i) {
            if ((j->second < i->second) || (j->second == i->second && disambiguateMax_)) {
                j = i;
            }
        }
        return j->first;
    }

    std::map<T, size_t> binCount_;
    bool disambiguateMax_ = true;
};


struct ModeInteger final : Mode<int> {
    using Mode::Mode;
    const char* name() const override { return "mode-integer"; }
    double value() const override { return binCount_.empty() ? missingValue() : static_cast<double>(mode()); }
    void count(const double& value) override {
        if (Counter::count(value)) {
            binCount_[static_cast<int>(std::lround(value))]++;
        }
    }
};


struct ModeRange final : Mode<size_t> {
    ModeRange(const param::MIRParametrisation& param) : Mode(param) {
        param.get("mode-range-min", mins_);
        param.get("mode-range-values", values_);

        ASSERT(!mins_.empty());

        if (values_.size() != mins_.size() + 1) {
            throw exception::UserError(
                "ModeRange: size mismatch: N+1 = #mode-range-values = " + std::to_string(values_.size()) +
                ", N = #mode-range-min = " + std::to_string(mins_.size()));
        }

        if (!std::is_sorted(values_.begin(), values_.end()) || !std::is_sorted(mins_.begin(), mins_.end())) {
            throw exception::UserError("ModeRange: mode-range-values and mode-range-min most be sorted");
        }
    }

    const char* name() const override { return "mode-range"; }

    double value() const override {
        if (binCount_.empty()) {
            return missingValue();
        }

        auto bin = mode();
        ASSERT(bin < values_.size());
        return values_[bin];
    }

    void count(const double& value) override {
        if (Counter::count(value)) {
            size_t bin = 0;
            while (bin < mins_.size() && mins_[bin] < value) {
                ++bin;
            }
            binCount_[bin]++;
        }
    }

    std::vector<double> mins_{0.5};
    std::vector<double> values_{0, 1};
};


struct ModeBoolean {
    ModeBoolean(const param::MIRParametrisation& param) {
        param.get("mode-disambiguate-max", disambiguateMax_);
        param.get("mode-boolean-min", min_);
    }

    const char* name() const { return "mode-boolean"; }

    double value() const { return majority_ > 0 || (majority_ == 0 && disambiguateMax_) ? 1 : 0; }

    void count(const double& value) {
        if (min_ <= value) {
            majority_++;
        }
        else {
            majority_--;
        }
    }

    void reset(double /*missingValue*/, bool /*hasMissing*/) { majority_ = 0; }

    long long majority_;
    double min_           = 0.5;
    bool disambiguateMax_ = true;
};


template <typename T>
StatisticsT<T>::StatisticsT(const param::MIRParametrisation& param) : VoronoiMethod(param) {
    setSolver(new SolveStatisticsT<T>(param));
}


static MethodBuilder<StatisticsT<Maximum>> __builder_1("voronoi-maximum");
static MethodBuilder<StatisticsT<Minimum>> __builder_2("voronoi-minimum");
static MethodBuilder<StatisticsT<CountAboveUpperLimit>> __builder_3("voronoi-count-above-upper-limit");
static MethodBuilder<StatisticsT<CountBelowLowerLimit>> __builder_4("voronoi-count-below-lower-limit");
static MethodBuilder<StatisticsT<Count>> __builder_5("voronoi-count");

static MethodBuilder<StatisticsT<Mean>> __builder_6("voronoi-mean");
static MethodBuilder<StatisticsT<Variance>> __builder_7("voronoi-variance");
static MethodBuilder<StatisticsT<Skewness>> __builder_8("voronoi-skewness");
static MethodBuilder<StatisticsT<Kurtosis>> __builder_9("voronoi-kurtosis");
static MethodBuilder<StatisticsT<StandardDeviation>> __builder_10("voronoi-stddev");

static MethodBuilder<StatisticsT<ModeInteger>> __builder_11("voronoi-mode-integer");
static MethodBuilder<StatisticsT<ModeRange>> __builder_12("voronoi-mode-range");


}  // namespace voronoi
}  // namespace method
}  // namespace mir
