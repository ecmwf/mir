// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/method/knn/distance/Kernel.h"

#include <cmath>
#include <ostream>

#include "mir/param/DefaultParametrisation.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/Exceptions.h"


namespace mir::method::knn::distance {


namespace {


double dot(const Point3& a, const Point3& b) {
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}


Point3 normalise(const Point3& a) {
    const auto n = std::sqrt(dot(a, a));
    ASSERT(n > 0.);
    return {a[0] / n, a[1] / n, a[2] / n};
}


}  // namespace


KernelProfile::KernelProfile(const std::string& name) : name_(name) {
    type_ = name == "gaussian"       ? Type::Gaussian
            : name == "top-hat"      ? Type::TopHat
            : name == "hann"         ? Type::Hann
            : name == "epanechnikov" ? Type::Epanechnikov
                                     : throw exception::UserError("KernelProfile: unknown kernel '" + name + "'");
}


double KernelProfile::operator()(double s) const {
    ASSERT(s >= 0.);

    switch (type_) {
        case Type::Gaussian:
            return std::exp(-0.5 * s * s);
        case Type::TopHat:
            return s <= 1. ? 1. : 0.;
        case Type::Hann:
            return s < 1. ? 0.5 * (1. + std::cos(M_PI * s)) : 0.;
        case Type::Epanechnikov:
            return s < 1. ? 1. - s * s : 0.;
    }

    NOTIMP;
}


double KernelProfile::at_distance2(double r2, double h) const {
    ASSERT(h > 0.);
    return type_ == Type::Gaussian ? std::exp(r2 * (-1. / (2. * h * h))) : operator()(std::sqrt(r2) / h);
}


bool KernelProfile::sameAs(const KernelProfile& other) const {
    return type_ == other.type_;
}


void KernelProfile::print(std::ostream& out) const {
    out << name_;
}


double kernel_radius(const param::MIRParametrisation& param, const std::string& key) {
    auto radius = param::DefaultParametrisation::instance().get_value<double>("distance", param);
    param.get(key, radius);
    if (!(radius > 0.)) {
        throw exception::UserError("Kernel: '" + key + "' should be positive");
    }
    return radius;
}


TangentPlane::TangentPlane(const Point3& p) : origin(p) {
    const auto up = normalise(p);
    east          = std::hypot(up[0], up[1]) > 1e-12 ? normalise({-up[1], up[0], 0.}) : Point3{0., 1., 0.};
    north         = Point3::cross(up, east);
}


std::pair<double, double> TangentPlane::operator()(const Point3& q) const {
    const Point3 d{q[0] - origin[0], q[1] - origin[1], q[2] - origin[2]};
    return {dot(d, east), dot(d, north)};
}


}  // namespace mir::method::knn::distance
