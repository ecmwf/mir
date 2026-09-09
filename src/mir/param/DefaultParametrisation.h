// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


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

        if constexpr (std::is_same_v<std::remove_cv_t<T>, size_t>) {
            ASSERT(MIRParametrisation::get(name, value));
        }
        else {
            ASSERT(get(name, value));
        }
        param.get(name, value);

        return value;
    }

    template <typename T>
    void json(eckit::JSON& j, const std::string& name, const T& value) const {
        T default_value{};

        if constexpr (std::is_floating_point_v<std::remove_cv_t<T>>) {
            if (!get(name, default_value) || !eckit::types::is_approximately_equal(value, default_value)) {
                j << name << value;
            }
        }
        else if constexpr (std::is_same_v<std::remove_cv_t<T>, size_t>) {
            if (!MIRParametrisation::get(name, default_value) || value != default_value) {
                j << name << value;
            }
        }
        else if (!get(name, default_value) || value != default_value) {
            j << name << value;
        }
    }
};


}  // namespace mir::param
