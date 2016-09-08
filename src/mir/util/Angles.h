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
#include "mir/util/Compare.h"


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


/// @return degree from a complex number
template< typename T>
T convert_complex_to_degrees(const std::complex<T>& c) {
    if ( eckit::FloatCompare<T>::isApproximatelyEqual(c.real(), 0) &&
         eckit::FloatCompare<T>::isApproximatelyEqual(c.imag(), 0) ) {
        return T(0);
    }
    // [-π, π] -> [-180°, 180°]
    return radian_to_degree(std::arg(c));
}


/// @return complex number from an angle [°]
template< typename T>
static std::complex<T> convert_degrees_to_complex(const T& theta) {
    return std::polar<T>(1, degree_to_radian(theta));
}


/// Convert from Cartesian 2D to Polar representation radius and angle [°] in [0°, 360°[ range
struct ConvertVectorCartesian2dToPolarDegrees {
    const compare::IsMissingFn isMissing_;

    ConvertVectorCartesian2dToPolarDegrees(const double& missingValue) :
        isMissing_(missingValue) {}

    double r(const double& x, const double& y) {
        if (isMissing_(x) || isMissing_(y)) {
            return isMissing_.missingValue_;
        }
        return std::sqrt(x*x + y*y);
    }

    double theta(const double& x, const double& y) {
        if (isMissing_(x) || isMissing_(y)) {
            return isMissing_.missingValue_;
        }
        else if (compare::is_approx_zero(x) && compare::is_approx_zero(y)) {
            return 0.;
        }
        // [-π, π] -> [-180°, 180°]
        return radian_to_degree(std::atan2(y, x));
    }

};


}  // namespace angles
}  // namespace util
}  // namespace mir


#endif

