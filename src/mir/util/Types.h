// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <vector>

#include "eckit/geometry/Point2.h"
#include "eckit/geometry/Point3.h"

#include "mir/util/Latitude.h"
#include "mir/util/LongitudeDouble.h"
#include "mir/util/LongitudeFraction.h"
#include "mir/util/PointLatLonT.h"


namespace mir {


// using Longitude = LongitudeDouble;
using Longitude   = LongitudeFraction;
using PointLatLon = util::PointLatLonT<Latitude, Longitude>;

using eckit::geometry::Point2;
using eckit::geometry::Point3;

using LLCOORDS  = eckit::geometry::LLCOORDS;
using XYZCOORDS = eckit::geometry::XYZCOORDS;

using MIRValuesVector = std::vector<double>;


}  // namespace mir
