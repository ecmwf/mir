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

#include "eckit/geo/area/BoundingBox.h"
#include "eckit/geo/figure/Earth.h"
#include "eckit/geo/geometry/Sphere.h"

#include "mir/util/Types.h"


namespace mir::util {


struct Earth : eckit::geo::geometry::Sphere {
    static double radius() { return eckit::geo::figure::DatumIFS::radius; }

    static double distance(const PointLonLat& a, const PointLonLat& b) { return Sphere::distance(radius(), a, b); }

    static PointLonLat convertCartesianToSpherical(const PointXYZ& p) {
        return Sphere::convertCartesianToSpherical(radius(), p);
    }

    static PointXYZ convertSphericalToCartesian(const PointLonLat& p, double height = 0.) {
        return Sphere::convertSphericalToCartesian(radius(), p, height);
    }

    static double distance(const PointXYZ& a, const PointXYZ& b) { return Sphere::distance(radius(), a, b); }

    static double area(double north, double west, double south, double east) {
        return Sphere::area(radius(), {north, west, south, east});
    }
};


}  // namespace mir::util
