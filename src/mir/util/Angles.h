/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @date Aug 2016


#ifndef mir_util_Angles_H
#define mir_util_Angles_H

#include <cmath>


namespace mir {
namespace util {
namespace angles {


/// @return degree from radian (no range check)
inline double degree_to_radian(const double& a) {
    return a * (M_PI / 180.0);
}


}  // namespace angles
}  // namespace util
}  // namespace mir


#endif

