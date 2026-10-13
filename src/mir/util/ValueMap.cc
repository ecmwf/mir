// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/util/ValueMap.h"

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

#include "eckit/config/Configured.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/log/JSON.h"
#include "eckit/value/Value.h"

#include "mir/param/SimpleParametrisation.h"


namespace mir::util {


namespace {
// NOTE: maps are encoded as strings (JSON) as SimpleParametrisation is flat; eckit::geo::Spec can nest.
std::string to_json_string(const eckit::ValueMap& map) {
    std::ostringstream ss;
    eckit::JSON j(ss);
    j << map;

    return ss.str();
}


template <typename Config>
void set_value(Config& config, const std::string& k, const eckit::Value& v) {
    if (v.isList()) {
        // lists of numbers or strings (as vectors)
        const eckit::ValueList list(v);
        auto all = [&list](auto pred) { return std::all_of(list.begin(), list.end(), pred); };

        if (all([](const eckit::Value& x) { return x.isNumber(); })) {
            config.set(k, std::vector<long>(list.begin(), list.end()));
        }
        else if (all([](const eckit::Value& x) { return x.isNumber() || x.isDouble(); })) {
            config.set(k, std::vector<double>(list.begin(), list.end()));
        }
        else if (all([](const eckit::Value& x) { return x.isString(); })) {
            config.set(k, std::vector<std::string>(list.begin(), list.end()));
        }
        else {
            throw eckit::BadValue("ValueMap: '" + k + "' list of numbers or strings expected", Here());
        }
        return;
    }

    v.isDouble()   ? config.set(k, v.as<double>())
    : v.isNumber() ? config.set(k, static_cast<long>(v))
    : v.isBool()   ? config.set(k, v.as<bool>())
    : v.isMap()    ? config.set(k, to_json_string(v))
                   : config.set(k, v.as<std::string>());
}


}  // namespace


ValueMap::ValueMap(const eckit::Value& value) : eckit::ValueMap(value) {}


void ValueMap::set(eckit::Configured& config) const {
    for (const auto& [k, v] : *this) {
        set_value(config, k, v);
    }
}


void ValueMap::set(param::SimpleParametrisation& config) const {
    for (const auto& [k, v] : *this) {
        set_value(config, k, v);
    }
}


}  // namespace mir::util
