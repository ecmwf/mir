// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <cmath>

#include "eckit/geo/PointLonLat.h"
#include "eckit/types/FloatCompare.h"


namespace mir::util {


/// @return degree to radian (no range check)
inline double degree_to_radian(const double& a) {
    return a * (M_PI / 180.0);
}


/// @return radian to degree (no range check)
inline double radian_to_degree(const double& a) {
    return a * (M_1_PI * 180.);
}


/// @return longitude in degree within range [minimum, minimum + 360[, those within PointLonLat::EPS short of
/// minimum + 360 as minimum (as the comparisons of coordinates)
inline double normalise_longitude(const double& lon, const double& minimum) {
    using eckit::geo::PointLonLat;
    const auto l = PointLonLat::normalise_angle_to_minimum(lon, minimum);
    return eckit::types::is_approximately_equal(l, minimum + PointLonLat::FULL_ANGLE, PointLonLat::EPS) ? minimum : l;
}


}  // namespace mir::util
