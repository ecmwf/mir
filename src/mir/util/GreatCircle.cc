/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/util/GreatCircle.h"

#include <cmath>
#include "eckit/types/FloatCompare.h"
#include "atlas/util/Constants.h"
#include "mir/util/Angles.h"


namespace mir {
namespace util {


static double R = atlas::util::Earth::radiusInMeters();


double GreatCircle::centralAngle(const point_ll_t& p1, const point_ll_t& p2) {

    // notice the order
    const point_2d_t q1(p1.lat.value(), p1.lon.value());
    const point_2d_t q2(p2.lat.value(), p2.lon.value());

    return centralAngle(q1, q2);
}


double GreatCircle::centralAngle(const point_2d_t& p1, const point_2d_t& p2) {

    /*
     * Δσ = atan( ((cos(ϕ2) * sin(Δλ))^2 + (cos(ϕ1) * sin(ϕ2) - sin(ϕ1) * cos(ϕ2) * cos(Δλ))^2) /
     *            (sin(ϕ1) * sin(ϕ2) + cos(ϕ1) * cos(ϕ2) * cos(Δλ)) )
     *
     * @article{doi:10.1179/sre.1975.23.176.88,
     * author = {T. Vincenty},
     * title = {Direct and Inverse Solutions of Geodesics on the Ellipsoid With Application of Nested Equations},
     * journal = {Survey Review},
     * volume = {23},
     * number = {176},
     * pages = {88-93},
     * year = {1975},
     * doi = {10.1179/sre.1975.23.176.88}
     * }
     */

    // notice the order
    ASSERT(-90. <= p1[0] && p1[0] <= 90.);
    ASSERT(-90. <= p2[0] && p2[0] <= 90.);
    const double
            phi1 = angles::degree_to_radian(p1[0]),
            phi2 = angles::degree_to_radian(p2[0]),
            lambda = angles::degree_to_radian(std::abs(angles::between_m180_and_p180(p2[1] - p1[1]))),

            cos_phi1 = std::cos(phi1),
            cos_phi2 = std::cos(phi2),
            sin_phi1 = std::sin(phi1),
            sin_phi2 = std::sin(phi2),
            cos_lambda = std::cos(lambda),
            sin_lambda = std::sin(lambda),

            angle = std::atan2(
                std::sqrt(std::pow(cos_phi2 * sin_lambda, 2) + std::pow(cos_phi1 * sin_phi2 - sin_phi1 * cos_phi2 * cos_lambda, 2)),
                sin_phi1 * sin_phi2 + cos_phi1 * cos_phi2 * cos_lambda );

    if (eckit::types::is_approximately_equal(angle, 0.)) {
        return 0.;
    }

    ASSERT(angle > 0.);
    return angle;
}


double GreatCircle::centralAngle(const point_3d_t& p1, const point_3d_t& p2) {

    // Δσ = 2 * asin( chord / 2 )

    const double d2 = point_3d_t::distance2(p1, p2);
    if (eckit::types::is_approximately_equal(d2, 0.)) {
        return 0.;
    }

    const double
            chord = std::sqrt(d2) / R,
            angle = std::asin(chord * 0.5) * 2.;

    return angle;
}


double GreatCircle::distanceInMeters(const point_ll_t& p1, const point_ll_t& p2) {
    return R * centralAngle(p1, p2);
}


double GreatCircle::distanceInMeters(const point_2d_t& p1, const point_2d_t& p2) {
    return R * centralAngle(p1, p2);
}


double GreatCircle::distanceInMeters(const point_3d_t& p1, const point_3d_t& p2) {
    return R * centralAngle(p1, p2);
}


}  // namespace util
}  // namespace mir

