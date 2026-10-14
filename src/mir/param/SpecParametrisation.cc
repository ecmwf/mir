// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/param/SpecParametrisation.h"

#include <regex>

#include "eckit/parser/YAMLParser.h"
#include "eckit/utils/StringTools.h"

#include "mir/util/ValueMap.h"


namespace mir::param {


SpecParametrisation::SpecParametrisation(const std::string& spec, const std::string& key) :
    CombinedParametrisation(spec_user_) {
    parse(spec, key).set(spec_user_);
}


util::ValueMap SpecParametrisation::parse(const std::string& spec, const std::string& key) {
    util::ValueMap map;

    if (auto trimmed = eckit::StringTools::trim(spec); trimmed.empty()) {
        return map;
    }
    else if (trimmed.front() != '{') {
        map[key] = trimmed;
        return map;
    }

    // keys: type as key, and a_b as a-b
    for (const auto& [k, v] : util::ValueMap(eckit::YAMLParser::decodeString(spec))) {
        const auto s                                                         = k.as<std::string>();
        map[s == "type" ? key : std::regex_replace(s, std::regex("_"), "-")] = v;
    }

    return map;
}


}  // namespace mir::param
