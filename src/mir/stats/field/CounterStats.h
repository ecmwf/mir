// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/stats/Field.h"
#include "mir/stats/detail/Counter.h"


namespace mir::stats::field {


/// Counting statistics on values (min, max, etc.)
struct CounterStats : detail::Counter, Field {
    CounterStats(const param::MIRParametrisation& param) : Counter(param), Field(param) {}

    double value() const override            = 0;
    void json(eckit::JSON&) const override   = 0;
    void print(std::ostream&) const override = 0;

    void count(const double& value) override { Counter::count(value); }
    void reset(double missingValue, bool hasMissing) override { Counter::reset(missingValue, hasMissing); }
};


}  // namespace mir::stats::field
