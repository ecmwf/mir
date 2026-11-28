// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/grib/OutputKeys.h"

#include <algorithm>

#include "mir/util/Grib.h"


namespace mir::grib {


namespace {


void add(OutputKeys::keys_type& keys, const std::string& key, const OutputKeys::value_type& value) {
    auto it = std::find_if(keys.begin(), keys.end(), [&key](const auto& kv) { return kv.first == key; });
    if (it != keys.end()) {
        it->second = value;
        return;
    }
    keys.emplace_back(key, value);
}


void set(grib_handle* h, const OutputKeys::keys_type& keys) {
    for (const auto& [key, value] : keys) {
        if (const auto* l = std::get_if<long>(&value)) {
            GRIB_CALL(codes_set_long(h, key.c_str(), *l));
        }
        else if (const auto* d = std::get_if<double>(&value)) {
            GRIB_CALL(codes_set_double(h, key.c_str(), *d));
        }
        else {
            const auto& s = std::get<std::string>(value);
            auto length   = s.length();
            GRIB_CALL(codes_set_string(h, key.c_str(), s.c_str(), &length));
        }
    }
}


}  // namespace


void OutputKeys::add_before(const std::string& key, const value_type& value) {
    add(before_, key, value);
}


void OutputKeys::add_after(const std::string& key, const value_type& value) {
    add(after_, key, value);
}


void OutputKeys::set_before(grib_handle* h) const {
    set(h, before_);
}


void OutputKeys::set_after(grib_handle* h) const {
    set(h, after_);
}


void OutputKeys::fill(grib_info& info) const {
    for (const auto* keys : {&before_, &after_}) {
        for (const auto& [key, value] : *keys) {
            if (key == "deleteLocalDefinition") {
                info.packing.deleteLocalDefinition = std::get<long>(value);
            }
            else if (const auto* l = std::get_if<long>(&value)) {
                info.extra_set(key.c_str(), *l);
            }
            else if (const auto* d = std::get_if<double>(&value)) {
                info.extra_set(key.c_str(), *d);
            }
            else {
                info.extra_set(key.c_str(), std::get<std::string>(value).c_str());
            }
        }
    }
}


}  // namespace mir::grib
