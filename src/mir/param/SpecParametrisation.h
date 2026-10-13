// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <exception>
#include <initializer_list>
#include <map>
#include <memory>
#include <string>
#include <type_traits>

#include "mir/param/CombinedParametrisation.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/util/Log.h"
#include "mir/util/ValueMap.h"


namespace mir::param {


namespace detail {
struct SpecUserParametrisation {
    SimpleParametrisation spec_user_;
};
}  // namespace detail


/**
 * @brief Parametrisation from a spec, as user parametrisation (combined with the defaults)
 * @details A spec is a name, or a mapping {type: name, a_b: value, ...} (YAML or JSON), as user parametrisation
 * (key=name, a-b=value, ...), where key names the type (e.g. "interpolation", "action"). The user parametrisation is
 * owned, so objects built from a spec can own their parametrisation (they keep a reference to it).
 */
class SpecParametrisation final : private detail::SpecUserParametrisation, public CombinedParametrisation {
public:
    // -- Constructors

    SpecParametrisation(const std::string& spec, const std::string& key);

    // -- Class methods

    /// Spec as (key=name, a-b=value, ...)
    static util::ValueMap parse(const std::string& spec, const std::string& key);

    /// Specs (complete) of the objects built from their name alone, to their name (those available)
    template <typename Factory>
    static std::map<std::string, std::string> knownSpecs(std::initializer_list<const char*> names) {
        SimpleParametrisation empty;
        CombinedParametrisation param(empty);

        std::map<std::string, std::string> known;
        for (const std::string name : names) {
            try {
                using T = std::remove_pointer_t<decltype(Factory::build(name, param))>;
                std::unique_ptr<const T> object(Factory::build(name, param));
                known.emplace(object->json_str(false), name);
            }
            catch (const std::exception& e) {
                Log::debug() << "SpecParametrisation: '" << name << "' not available: " << e.what() << std::endl;
            }
        }

        return known;
    }
};


}  // namespace mir::param
