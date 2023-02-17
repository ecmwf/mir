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
