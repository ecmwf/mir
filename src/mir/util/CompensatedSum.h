// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <cmath>


namespace mir::util {


/**
 * @brief Compensated (Kahan-Babuska-Neumaier) summation: the rounding error does not grow with the number of terms
 *
 * A naive sum of n terms accumulates an error of up to about n epsilon (relative), e.g. 1e-13 for a few thousand
 * weights, beyond the tolerance of weights adding up to one; this sum stays within a few epsilon.
 */
template <typename T = double>
class CompensatedSum {
public:
    CompensatedSum& operator+=(T value) {
        const auto t = sum_ + value;
        compensation_ += std::abs(sum_) >= std::abs(value) ? (sum_ - t) + value : (value - t) + sum_;
        sum_ = t;
        return *this;
    }

    T value() const { return sum_ + compensation_; }

private:
    T sum_          = 0;
    T compensation_ = 0;
};


}  // namespace mir::util
