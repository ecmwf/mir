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

#include <vector>

#include "eckit/geometry/Point2.h"
#include "eckit/geometry/Point3.h"

#include "eckit/geo/PointLonLat.h"
#include "eckit/geo/PointXY.h"
#include "eckit/geo/PointXYZ.h"

#include "mir/util/Latitude.h"
#include "mir/util/LongitudeDouble.h"
#include "mir/util/LongitudeFraction.h"


namespace mir {


// using Longitude = LongitudeDouble;
using Longitude = LongitudeFraction;

using eckit::geo::PointLonLat;
using eckit::geo::PointXY;
using eckit::geo::PointXYZ;

using MIRValuesVector = std::vector<double>;


}  // namespace mir
