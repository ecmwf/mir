// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "action.h"


namespace mir::action::tests {


inline size_t number_of_values(size_t T) {
    return (T + 1) * (T + 2);
}


/// Spectral field, the coefficient values as their position
inline data::MIRField spectral(size_t T, size_t dimensions = 1) {
    MIRValuesVector values(number_of_values(T));

    for (size_t i = 0; i < values.size(); ++i) {
        values[i] = static_cast<double>(i + 1);
    }

    return field("{grid: T" + std::to_string(T) + "}", values, dimensions);
}


}  // namespace mir::action::tests
