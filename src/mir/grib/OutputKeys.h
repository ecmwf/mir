// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <string>
#include <utility>
#include <variant>
#include <vector>


struct grib_handle;
struct grib_info;


namespace mir::grib {


/// GRIB keys of the output other than its geometry (gridSpec) and values, set before the geometry (those it encodes
/// with) or after it (as setting the geometry resets them)
class OutputKeys {
public:
    // -- Types

    using value_type = std::variant<long, double, std::string>;
    using keys_type  = std::vector<std::pair<std::string, value_type>>;

    // -- Methods

    /// Add a key (replacing its value, if added already)
    void add_before(const std::string& key, const value_type&);
    void add_after(const std::string& key, const value_type&);

    const keys_type& before() const { return before_; }
    const keys_type& after() const { return after_; }

    void set_before(grib_handle*) const;
    void set_after(grib_handle*) const;

    /// The keys as codes_grib_util_set_spec takes them, some as its own fields
    void fill(grib_info&) const;

private:
    // -- Members

    keys_type before_;
    keys_type after_;
};


}  // namespace mir::grib
