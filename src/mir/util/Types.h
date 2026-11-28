// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <vector>

#include "eckit/geo/PointLonLat.h"
#include "eckit/geo/PointXY.h"
#include "eckit/geo/PointXYZ.h"
#include "eckit/geo/figure/Earth.h"

#include "mir/util/Latitude.h"
#include "mir/util/LongitudeDouble.h"
#include "mir/util/LongitudeFraction.h"
#include "mir/util/PointLatLonT.h"


namespace mir {


// using Longitude = LongitudeDouble;
using Longitude   = LongitudeFraction;
using PointLatLon = util::PointLatLonT<Latitude, Longitude>;

using eckit::geo::PointLonLat;
using eckit::geo::PointXY;
using eckit::geo::PointXYZ;

inline constexpr double NORTH_POLE      = PointLonLat::RIGHT_ANGLE;
inline constexpr double SOUTH_POLE      = -PointLonLat::RIGHT_ANGLE;
inline constexpr double LATITUDE_GLOBE  = PointLonLat::FLAT_ANGLE;  // from pole to pole
inline constexpr double LONGITUDE_GLOBE = PointLonLat::FULL_ANGLE;  // around

using MIRValuesVector = std::vector<double>;


namespace util {
using Earth = eckit::geo::figure::Earth;
}


}  // namespace mir
