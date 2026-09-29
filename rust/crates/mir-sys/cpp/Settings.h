// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "rust/cxx.h"


namespace mir_bridge {


/**
 * The setters shared by `Job` and `Parametrisation`, forwarding to the `set`
 * and `clear` overloads each inherits from mir. They are named per type so
 * Rust passes `&str` and slices rather than building `CxxString` and
 * `CxxVector` at every call site.
 */
template <class Derived>
class Settings {
public:
    void set_str(rust::Str name, rust::Str value) { self().set(std::string(name), std::string(value)); }

    void set_f64(rust::Str name, double value) { self().set(std::string(name), value); }

    void set_i64(rust::Str name, int64_t value) { self().set(std::string(name), static_cast<long long>(value)); }

    void set_bool(rust::Str name, bool value) { self().set(std::string(name), value); }

    void set_f64_list(rust::Str name, rust::Slice<const double> values) {
        self().set(std::string(name), std::vector<double>(values.begin(), values.end()));
    }

    void set_i64_list(rust::Str name, rust::Slice<const int64_t> values) {
        self().set(std::string(name), std::vector<long long>(values.begin(), values.end()));
    }

    void set_str_list(rust::Str name, rust::Slice<const rust::Str> values) {
        std::vector<std::string> converted;
        converted.reserve(values.size());
        for (const auto& value : values) {
            converted.emplace_back(value);
        }
        self().set(std::string(name), converted);
    }

    void clear_key(rust::Str name) { self().clear(std::string(name)); }

private:
    Derived& self() { return static_cast<Derived&>(*this); }
};


}  // namespace mir_bridge
