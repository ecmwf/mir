// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <cmath>

#include "mir/util/LongitudeDouble.h"


namespace mir::util {


/// @return degree to radian (no range check)
inline double degree_to_radian(const double& a) {
    return a * (M_PI / 180.0);
}


/// @return radian to degree (no range check)
inline double radian_to_degree(const double& a) {
    return a * (M_1_PI * 180.);
}


/// @return longitude in degree within range [minimum, minimum + 360[
inline double normalise_longitude(const double& lon, const double& minimum) {
    return LongitudeDouble(lon).normalise(minimum).value();
}


}  // namespace mir::util
