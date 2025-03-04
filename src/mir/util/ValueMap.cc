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
