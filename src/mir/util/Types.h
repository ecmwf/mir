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

#include "eckit/geo/PointLonLat.h"
#include "eckit/geo/PointXY.h"
#include "eckit/geo/PointXYZ.h"


namespace mir {


using eckit::geo::PointLonLat;
using eckit::geo::PointXY;
using eckit::geo::PointXYZ;

using MIRValuesVector = std::vector<double>;


}  // namespace mir
