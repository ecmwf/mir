// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <cmath>
#include <iterator>
#include <memory>
#include <sstream>
#include <string>

#include "eckit/geo/Grid.h"
#include "eckit/log/JSON.h"
#include "eckit/testing/Test.h"

#include "mir/param/GridSpecParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Grib.h"


namespace mir::repres::tests {


/// Representation by its gridSpec
inline RepresentationHandle make(const std::string& gridspec) {
    param::GridSpecParametrisation param(gridspec);
    return RepresentationFactory::build(param);
}


/// Number of points of the values as held, by iterating
inline size_t count(const Representation& repres) {
    const auto& grid = repres.canonicalGrid();
    return static_cast<size_t>(std::distance(grid.cbegin(), grid.cend()));
}


inline std::string to_string(const Representation& repres) {
    std::ostringstream str;
    str << repres;
    return str.str();
}


inline std::string to_json(const Representation& repres) {
    std::ostringstream str;
    eckit::JSON j(str);
    j << repres;
    return str.str();
}


inline bool same(double a, double b, double eps = 1e-9) {
    return std::abs(a - b) <= eps;
}


/// Extra setting of a grib_info, by name (nullptr if not set)
inline const grib_values* extra(const grib_info& info, const std::string& name) {
    for (long j = 0; j < info.packing.extra_settings_count; ++j) {
        if (name == info.packing.extra_settings[j].name) {
            return &info.packing.extra_settings[j];
        }
    }
    return nullptr;
}


inline long extra_long(const grib_info& info, const std::string& name) {
    const auto* set = extra(info, name);
    ASSERT(set != nullptr && set->type == CODES_TYPE_LONG);
    return set->long_value;
}


inline double extra_double(const grib_info& info, const std::string& name) {
    const auto* set = extra(info, name);
    ASSERT(set != nullptr && set->type == CODES_TYPE_DOUBLE);
    return set->double_value;
}


inline std::string extra_string(const grib_info& info, const std::string& name) {
    const auto* set = extra(info, name);
    ASSERT(set != nullptr && set->type == CODES_TYPE_STRING);
    return set->string_value;
}


}  // namespace mir::repres::tests
