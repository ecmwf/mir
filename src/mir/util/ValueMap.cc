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

#include "eckit/config/Configured.h"

#include "mir/param/SimpleParametrisation.h"


namespace mir {
namespace util {


ValueMap::ValueMap(const eckit::Value& value) : eckit::ValueMap(value) {}


void ValueMap::set(eckit::Configured& config) const {
    for (const auto& kv : *this) {
        kv.second.isDouble()   ? config.set(kv.first, kv.second.as<double>())
        : kv.second.isNumber() ? config.set(kv.first, long(kv.second))
        : kv.second.isBool()   ? config.set(kv.first, kv.second.as<bool>())
                               : config.set(kv.first, kv.second.as<std::string>());
    }
}


void ValueMap::set(param::SimpleParametrisation& config) const {
    for (const auto& kv : *this) {
        kv.second.isDouble()   ? config.set(kv.first, kv.second.as<double>())
        : kv.second.isNumber() ? config.set(kv.first, long(kv.second))
        : kv.second.isBool()   ? config.set(kv.first, kv.second.as<bool>())
                               : config.set(kv.first, kv.second.as<std::string>());
    }
}


}  // namespace util
}  // namespace mir
