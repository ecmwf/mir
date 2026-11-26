// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <iosfwd>
#include <string>
#include <utility>

#include "mir/util/Types.h"


namespace mir::param {
class MIRParametrisation;
}


namespace mir::method::knn::distance {


/**
 * @brief Radial kernel profile K(s), of the normalised distance s = r / h (h the kernel radius), with K(0) = 1
 *
 * Profiles:
 * - gaussian:     exp(-s^2 / 2), h is the standard deviation
 * - top-hat:      1 for s <= 1, 0 otherwise
 * - hann:         (1 + cos(pi s)) / 2 for s < 1, 0 otherwise
 * - epanechnikov: 1 - s^2 for s < 1, 0 otherwise
 */
class KernelProfile {
public:
    explicit KernelProfile(const std::string& name);

    double operator()(double s) const;

    /// K(r / h), from the squared distance r^2 (exp(-r^2 / 2 h^2) for gaussian, as computed historically)
    double at_distance2(double r2, double h) const;

    const std::string& name() const { return name_; }

    bool sameAs(const KernelProfile&) const;
    void print(std::ostream&) const;

private:
    enum class Type
    {
        Gaussian,
        TopHat,
        Hann,
        Epanechnikov
    };

    std::string name_;
    Type type_;
};


/// Kernel radius h (option key, default "distance"), in metres
double kernel_radius(const param::MIRParametrisation&, const std::string& key = "kernel-radius");


/**
 * @brief Local tangent plane at a point (of a sphere centred at the origin), with axes east and north
 *
 * Offsets of nearby points are their projection on the plane, in the units of the coordinates (metres). Valid for
 * distances much smaller than the sphere radius. At the poles, east is the y axis.
 */
struct TangentPlane {
    explicit TangentPlane(const Point3&);

    /// Offset (east, north) of a point from the tangent point
    std::pair<double, double> operator()(const Point3&) const;

    Point3 origin;
    Point3 east;
    Point3 north;
};


}  // namespace mir::method::knn::distance
