// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <ostream>

#include "mir/stats/detail/CentralMomentsT.h"


namespace mir::stats::detail {


/// Scalar statistics (composition)
template <typename T>
struct ScalarT : public CentralMomentsT<T> {
    void print(std::ostream& out) const override {
        using t = CentralMomentsT<T>;
        out << "Scalar["
               "mean="
            << t::mean() << ",stddev=" << t::standardDeviation() << ",skewness=" << t::skewness()
            << ",kurtosis=" << t::kurtosis() << "]";
    }
};


}  // namespace mir::stats::detail
