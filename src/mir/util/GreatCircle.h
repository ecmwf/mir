/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#ifndef mir_util_GreatCircle_h
#define mir_util_GreatCircle_h

#include "mir/repres/Iterator.h"


namespace mir {
namespace util {


struct GreatCircle {

    typedef repres::Iterator::point_ll_t point_ll_t;
    typedef repres::Iterator::point_2d_t point_2d_t;
    typedef repres::Iterator::point_3d_t point_3d_t;

    // Great-circle central angle between two points, in radians from (Latitude, Longitude)
    static double centralAngle(const point_ll_t&, const point_ll_t&);

    // Great-circle central angle between two points, in radians from (Latitude, Longitude) real values
    static double centralAngle(const point_2d_t&, const point_2d_t&);

    // Great-circle central angle between two points, in radians from (X, Y, Z)
    static double centralAngle(const point_3d_t&, const point_3d_t&);

    // Great-circle distance between two points, in meters from (Latitude, Longitude)
    static double distanceInMeters(const point_ll_t&, const point_ll_t&);

    // Great-circle distance between two points, in meters from (Latitude, Longitude) real values
    static double distanceInMeters(const point_2d_t&, const point_2d_t&);

    // Great-circle distance between two points, in meters from (X, Y, Z)
    static double distanceInMeters(const point_3d_t&, const point_3d_t&);

};


}  // namespace util
}  // namespace mir


#endif

