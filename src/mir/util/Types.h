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

#include "eckit/geo/Point2.h"
#include "eckit/geo/Point3.h"
#include "eckit/geo/PointLonLat.h"

#include "mir/util/Latitude.h"
#include "mir/util/LongitudeDouble.h"
#include "mir/util/LongitudeFraction.h"
#include "mir/util/PointLatLonT.h"


namespace mir {


// using Longitude = LongitudeDouble;
using Longitude   = LongitudeFraction;
using PointLatLon = util::PointLatLonT<Latitude, Longitude>;

using eckit::geo::Point2;
using eckit::geo::Point3;
using eckit::geo::PointLonLat;

using MIRValuesVector = std::vector<double>;


}  // namespace mir
