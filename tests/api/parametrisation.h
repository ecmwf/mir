// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <cmath>
#include <sstream>
#include <string>

#include "eckit/testing/Test.h"

#include "mir/param/SimpleParametrisation.h"
#include "mir/util/Log.h"


namespace mir::api::tests {


using param::MIRParametrisation;
using param::SimpleParametrisation;


template <typename T>
std::string to_string(const T& p) {
    std::ostringstream str;
    str << p;
    return str.str();
}


inline std::string to_tool_string(const MIRParametrisation& p) {
    std::ostringstream str;
    str << eckit::setformat(Log::applicationFormat) << p;
    return str.str();
}


inline bool same(double a, double b) {
    return std::abs(a - b) < 1e-9;
}


}  // namespace mir::api::tests
