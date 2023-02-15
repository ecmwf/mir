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


#pragma once

#include <string>

#include "mir/util/Log.h"


namespace mir::util {


struct DeprecatedFunctionality {
    DeprecatedFunctionality(const std::string& msg, Log::Channel& = Log::warning());

private:
    static void message(const std::string& msg, Log::Channel& = Log::warning());
};


}  // namespace mir::util
