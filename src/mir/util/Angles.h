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
#include <complex>
#include "eckit/types/FloatCompare.h"


namespace mir {
namespace util {
namespace angles {


/// @return degree from radian (no range check)
inline double degree_to_radian(const double& a) {
    return a * (M_PI / 180.0);
}


/// @return radian from degree (no range check)
inline double radian_to_degree(const double& a) {
    return a * (M_1_PI * 180.);
}


/// @return normalised angle [°] in [0°, 360°[ range
inline double between_0_and_360(double a) {
    while (a >= 360) {
        a -= 360;
    }
    while (a < 0 ) {
        a += 360;
    }
    return a;
}


/// @return normalised angle [°] in [-180°, 180°] range
inline double between_m180_and_p180(double a) {
    while (a > 180) {
        a -= 360;
    }
    while (a < -180 ) {
        a += 360;
    }
    return a;
}


/// @return normalised angle [r] in [0, 2π[ range
inline double between_0_and_2PI(double a) {
    while (a >= M_2_PI) {
        a -= M_2_PI;
    }
    while (a < 0 ) {
        a += M_2_PI;
    }
    return a;
}


/// @return normalised angle [p] in [-π, π] range
inline double between_mPI_and_pPI(double a) {
    while (a > M_PI) {
        a -= M_2_PI;
    }
    while (a < -M_PI ) {
        a += M_2_PI;
    }
    return a;
}


/// @return angle [radian] from a complex number
template< typename T>
T convert_complex_to_radians(const std::complex<T>& c) {
    if ( eckit::types::is_approximately_equal<double>(c.real(), 0.) &&
         eckit::types::is_approximately_equal<double>(c.imag(), 0.) ) {
        return T(0);
    }
    // [-π, π] -> [-180°, 180°]
    return std::arg(c);
}


/// @return complex number from an angle [radian]
template< typename T>
static std::complex<T> convert_radians_to_complex(const T& theta) {
    // TODO optionaly operate on radius != 1
    return std::polar<T>(1, theta);
}


/// @return angle [°] from a complex number
template< typename T>
T convert_complex_to_degrees(const std::complex<T>& c) {
    // [-π, π] -> [-180°, 180°]
    return radian_to_degree(convert_complex_to_radians<T>(c));
}


/// @return complex number from an angle [°]
template< typename T>
static std::complex<T> convert_degrees_to_complex(const T& theta) {
    // TODO optionaly operate on radius != 1
    return convert_radians_to_complex<T>(degree_to_radian(theta));
}


}  // namespace angles
}  // namespace util
}  // namespace mir


#endif

