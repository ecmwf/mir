// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/util/ValueMap.h"

#include <sstream>

#include "eckit/config/Configured.h"
#include "eckit/log/JSON.h"

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
}  // namespace


ValueMap::ValueMap(const eckit::Value& value) : eckit::ValueMap(value) {}


void ValueMap::set(eckit::Configured& config) const {
    for (const auto& [k, v] : *this) {
        v.isDouble()   ? config.set(k, v.as<double>())
        : v.isNumber() ? config.set(k, static_cast<long>(v))
        : v.isBool()   ? config.set(k, v.as<bool>())
        : v.isMap()    ? config.set(k, to_json_string(v))
                       : config.set(k, v.as<std::string>());
    }
}


void ValueMap::set(param::SimpleParametrisation& config) const {
    for (const auto& [k, v] : *this) {
        v.isDouble()   ? config.set(k, v.as<double>())
        : v.isNumber() ? config.set(k, static_cast<long>(v))
        : v.isBool()   ? config.set(k, v.as<bool>())
        : v.isMap()    ? config.set(k, to_json_string(v))
                       : config.set(k, v.as<std::string>());
    }
}


}  // namespace mir::util
