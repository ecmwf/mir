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


#ifndef mir_method_detail_GeneralStatsT_h
#define mir_method_detail_GeneralStatsT_h

#include "mir/param/MIRParametrisation.h"
#include "mir/stats/detail/CentralMomentsT.h"
#include "mir/stats/detail/Counter.h"
#include "mir/stats/detail/ModeT.h"


namespace mir {
namespace method {
namespace detail {


/// General statistics on values (mean, stddev, mode, etc.)
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


struct ModeReal final : StatsT<stats::detail::ModeReal> {
    ModeReal(const param::MIRParametrisation& param) : StatsT(param) { setup(param); }
    double value() const override { return mode(); }
    const char* name() const override { return "mode-real"; }
};


struct ModeIntegral final : StatsT<stats::detail::ModeIntegral> {
    ModeIntegral(const param::MIRParametrisation& param) : StatsT(param) { setup(param); }
    double value() const override { return mode(); }
    const char* name() const override { return "mode-integral"; }
};


struct ModeBoolean final : StatsT<stats::detail::ModeBoolean> {
    ModeBoolean(const param::MIRParametrisation& param) : StatsT(param) { setup(param); }
    double value() const override { return mode(); }
    const char* name() const override { return "mode-boolean"; }
};


}  // namespace detail
}  // namespace method
}  // namespace mir


#endif
