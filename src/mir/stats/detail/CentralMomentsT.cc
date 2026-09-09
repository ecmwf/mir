// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/stats/detail/CentralMomentsT.h"

#include <complex>
#include <limits>


namespace mir::stats::detail {


template <>
std::complex<double> CentralMomentsT<std::complex<double>>::mean() const {
    using limits = std::numeric_limits<double>;
    static std::complex<double> NaN{limits::quiet_NaN(), limits::quiet_NaN()};
    return count_ < 1 ? NaN : M1_;
}


template <>
std::complex<float> CentralMomentsT<std::complex<float>>::mean() const {
    using limits = std::numeric_limits<float>;
    static std::complex<float> NaN{limits::quiet_NaN(), limits::quiet_NaN()};
    return count_ < 1 ? NaN : M1_;
}


}  // namespace mir::stats::detail
