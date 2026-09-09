// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/stats/Field.h"
#include "mir/stats/detail/Counter.h"


namespace mir::stats::field {


/// Central moment statistics on values (mean, stddev, etc.)
template <typename STATS>
struct CentralMomentStatsT : detail::Counter, Field, STATS {
    CentralMomentStatsT(const param::MIRParametrisation& param) : Counter(param), Field(param) {}
    ~CentralMomentStatsT() override = default;

    double value() const override            = 0;
    void json(eckit::JSON&) const override   = 0;
    void print(std::ostream&) const override = 0;

    void count(const double& value) override {
        if (Counter::count(value)) {
            STATS::operator()(value);
        }
    }

    void reset(double missingValue, bool hasMissing) override {
        Counter::reset(missingValue, hasMissing);
        STATS::reset();
    }
};


}  // namespace mir::stats::field
