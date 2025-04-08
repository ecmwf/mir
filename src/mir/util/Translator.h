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

#include <algorithm>
#include <charconv>
#include <string>
#include <type_traits>

#include "mir/util/Exceptions.h"


namespace mir::util {


// NOTE: temporary code until the move to eckit SafeCasts


template <typename T, std::enable_if_t<!std::is_arithmetic_v<T>, int> = 0>
inline T from_string(const std::string&) {
    NOTIMP;
}


template <typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
inline T from_string(const std::string& from) {
    return static_cast<T>(std::stod(from));
}


template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
inline T from_string(const std::string& from) {
    T value = 0;

    if constexpr (std::is_unsigned_v<T>) {
        if (from.find('-') != std::string::npos) {
            throw exception::BadValue("from_string: negative to unsigned, from: '" + from + "'");
        }
    }

    auto [ptr, ec] = std::from_chars(from.data(), from.data() + from.size(), value);

    if (ec == std::errc::invalid_argument) {
        throw exception::BadValue("from_string: not a number, from: '" + from + "'");
    }

    if (ec == std::errc::result_out_of_range) {
        throw exception::BadValue("from_string: out of range, from: '" + from + "'");
    }

    if (ec != std::errc()) {
        throw exception::BadValue("from_string: failed to convert to number, from: '" + from + "'");
    }

    return value;
}


template <>
inline bool from_string<bool>(const std::string& from) {
    auto lower = [](const std::string& v) {
        std::string r = v;
        std::transform(r.begin(), r.end(), r.begin(), static_cast<int (*)(int)>(tolower));
        return r;
    };

    auto s = lower(from);

    if (s == "no" || s == "off" || s == "false" || s == "0") {
        return false;
    }

    if (s == "yes" || s == "on" || s == "true" || s != "0") {
        return true;
    }

    return from_string<long>(from) != 0;
}

}  // namespace mir::util
