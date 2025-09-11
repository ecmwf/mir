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

#include <type_traits>

#include "eckit/log/JSON.h"
#include "eckit/types/FloatCompare.h"

#include "mir/param/SimpleParametrisation.h"
#include "mir/util/Exceptions.h"


namespace mir::param {


class DefaultParametrisation : public SimpleParametrisation {
private:
    // -- Constructors

    DefaultParametrisation();

public:
    // -- Methods

    static const DefaultParametrisation& instance() {
        static const DefaultParametrisation instance;
        return instance;
    }

    template <typename T>
    T get_value(const std::string& name, const MIRParametrisation& param) const {
        T value{};
        ASSERT(get(name, value));  // ensure there is a default value
        param.get(name, value);    // override the default value if any
        return value;
    }

    template <>
    size_t get_value(const std::string& name, const MIRParametrisation& param) const {
        size_t value{};
        ASSERT(MIRParametrisation::get(name, value));  // ensure there is a default value
        param.get(name, value);                        // override the default value if any
        return value;
    }

    template <typename T>
    void json(eckit::JSON& j, const std::string& name, const T& value) const {
        if (std::remove_cv_t<T> default_value; !SimpleParametrisation::get(name, default_value)) {
            j << name << value;
        }
        else if constexpr (std::is_floating_point_v<std::remove_cv_t<T>>) {
            if (!eckit::types::is_approximately_equal(value, default_value)) {
                j << name << value;
            }
        }
        else if (value != default_value) {
            j << name << value;
        }
    }

    template <>
    void json(eckit::JSON& j, const std::string& name, const size_t& value) const {
        if (size_t default_value = 0; !MIRParametrisation::get(name, default_value) || value != default_value) {
            j << name << value;
        }
    }
};


}  // namespace mir::param
