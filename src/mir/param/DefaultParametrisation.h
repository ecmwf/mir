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
