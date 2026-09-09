// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <string>

#include "mir/util/Log.h"


namespace mir::util {


struct UnsupportedFunctionality {
    explicit UnsupportedFunctionality(const std::string& msg, Log::Channel& = Log::warning());

private:
    static void message(const std::string& msg, Log::Channel& = Log::warning());
};


}  // namespace mir::util
