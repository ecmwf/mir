// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <cmath>

#include "mir/util/Angles.h"
#include "mir/util/Log.h"

#include "action.h"


namespace mir::action::tests {


const std::string ROTATED{"{grid: [1, 1], area: [10, -10, -10, 10], rotation: [-40, 22]}"};


inline MIRValuesVector values(const std::string& grid, const F& f) {
    return field(grid, f).values(0);
}


/// Vector field, components u and v
inline data::MIRField uv(const std::string& grid, const F& u, const F& v) {
    auto f  = field(grid, u, 2);
    auto vv = values(grid, v);
    f.update(vv, 1, true);
    return f;
}


const double R = util::Earth::radius();


inline double sind(double x) {
    return std::sin(util::degree_to_radian(x));
}


inline double cosd(double x) {
    return std::cos(util::degree_to_radian(x));
}


/// Check field component against expected values, away from the poles (where the finite volume method is less
/// accurate), within tolerance
inline void expect_near(const data::MIRField& field, size_t which, const F& expected, double tolerance) {
    size_t n = 0;

    const auto& points = field.representation()->canonicalGridUnrotated();
    for (auto it = points.cbegin(); it; ++it) {
        const auto p   = std::get<PointLonLat>(*it);
        const auto lat = p.lat();

        if (std::abs(lat) < 80.) {
            const auto v = field.values(which)[it.index()];
            const auto e = expected(lat, p.lon());

            if (!same(v, e, tolerance) && n++ < 5) {
                Log::info() << "lat=" << lat << ": " << v << " (expected " << e << ")" << std::endl;
            }
        }
    }

    EXPECT_EQUAL(n, 0);
}


const std::string GRID{"{grid: O32}"};


}  // namespace mir::action::tests
