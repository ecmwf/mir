// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <algorithm>
#include <functional>
#include <memory>
#include <string>
#include <utility>

#include "mir/util/BoundingBox.h"

#include "action.h"


namespace mir::action::tests {


const F LATITUDE      = [](double lat, double) { return lat; };
const F CONSTANT      = [](double, double) { return 3.; };
const F MISSING_NORTH = [](double lat, double) { return lat > 45. ? 9999. : lat; };  // missing value 9999


/// Visit field points (geographic, ie. rotated if the grid is) and values
inline void for_each_point(const data::MIRField& field, size_t which,
                           const std::function<void(double lat, double lon, double value)>& f) {
    const auto& values = field.values(which);

    const auto& points = field.representation()->canonicalGrid();
    for (auto it = points.cbegin(); it; ++it) {
        const auto p = std::get<PointLonLat>(*it);
        f(p.lat(), p.lon(), values.at(it.index()));
    }
}


/// Largest difference of (non-missing) values to f, at points within a latitude band
inline double max_error(const data::MIRField& field, const F& f, double maxAbsLat = 90.) {
    double err = 0.;
    for_each_point(field, 0, [&](double lat, double lon, double v) {
        if (v != field.missingValue() && std::abs(lat) <= maxAbsLat) {
            err = std::max(err, std::abs(v - f(lat, lon)));
        }
    });
    return err;
}


struct count_t {
    size_t points  = 0;
    size_t missing = 0;
};


/// Number of points satisfying a latitude condition, and of those with missing values
inline count_t count_missing_if(const data::MIRField& field, const std::function<bool(double)>& condition) {
    count_t count;
    for_each_point(field, 0, [&](double lat, double, double v) {
        if (condition(lat)) {
            ++count.points;
            count.missing += v == field.missingValue() ? 1 : 0;
        }
    });
    return count;
}


/// Field points (geographic) are all in the bounding box
inline bool contained(const data::MIRField& field, const util::BoundingBox& bbox) {
    bool in = true;
    for_each_point(field, 0, [&](double lat, double lon, double) { in = in && bbox.contains(PointLatLon(lat, lon)); });
    return in;
}


/// Interpolation of a constant field, a field linear in latitude (within tolerance), and of missing values
inline void check_values(const action::Action& action, const std::string& input = "{grid: O16}", double tolerance = 0.2,
                         double maxAbsLat = 80.) {
    auto constant = perform(action, field(input, CONSTANT));

    EXPECT(!constant.hasMissing());
    EXPECT(max_error(constant, CONSTANT) < 1e-9);

    auto linear = perform(action, field(input, LATITUDE, 2));

    EXPECT_EQUAL(linear.dimensions(), 2);
    EXPECT(linear.values(0) == linear.values(1));
    EXPECT(max_error(linear, LATITUDE, maxAbsLat) < tolerance);

    // missing values (north of 45N) are interpolated as missing, other values are not
    auto missing = perform(action, field(input, MISSING_NORTH, 1, 9999.));

    EXPECT_EQUAL(missing.missingValue(), 9999.);

    const auto north = count_missing_if(missing, [](double lat) { return lat > 55.; });

    EXPECT(north.missing == north.points);
    EXPECT(missing.hasMissing() == (north.missing > 0));

    const auto south = count_missing_if(missing, [](double lat) { return lat < 35.; });

    EXPECT(south.missing == 0);
    EXPECT(max_error(missing, LATITUDE, 35.) < tolerance);
}


}  // namespace mir::action::tests
