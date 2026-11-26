// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/method/knn/distance/DirectionalKernel.h"

#include <cmath>
#include <ostream>
#include <sstream>
#include <string>

#include "eckit/log/JSON.h"
#include "eckit/types/FloatCompare.h"
#include "eckit/utils/MD5.h"

#include "mir/param/MIRParametrisation.h"
#include "mir/util/CompensatedSum.h"
#include "mir/util/Exceptions.h"


namespace mir::method::knn::distance {


namespace {


std::string profile_name(const param::MIRParametrisation& param) {
    std::string name = "gaussian";
    param.get("kernel", name);
    return name;
}


double kernel_angle(const param::MIRParametrisation& param) {
    double angle = 0.;
    param.get("kernel-angle", angle);
    return angle;
}


}  // namespace


DirectionalKernel::DirectionalKernel(const param::MIRParametrisation& param) :
    profile_(profile_name(param)),
    along_(kernel_radius(param)),
    across_(along_),
    angle_(kernel_angle(param)),
    cos_(std::cos(angle_ * M_PI / 180.)),
    sin_(std::sin(angle_ * M_PI / 180.)) {
    param.get("kernel-radius-across", across_);
    if (!(across_ > 0.)) {
        throw exception::UserError("DirectionalKernel: 'kernel-radius-across' should be positive");
    }
}


void DirectionalKernel::operator()(size_t ip, const Point3& point,
                                   const std::vector<search::PointSearch::PointValueType>& neighbours,
                                   std::vector<WeightMatrix::Triplet>& triplets) const {
    ASSERT(!neighbours.empty());

    triplets.clear();
    triplets.reserve(neighbours.size());

    const TangentPlane plane(point);

    util::CompensatedSum<double> total;  // the weights add up to one, even for many neighbours
    for (const auto& n : neighbours) {
        const auto [dx, dy] = plane(n.point());

        // offset along and across the kernel direction, scaled by the radii
        const auto u = (cos_ * dx + sin_ * dy) / along_;
        const auto v = (-sin_ * dx + cos_ * dy) / across_;

        const auto w = profile_(std::sqrt(u * u + v * v));
        if (w > 0.) {
            triplets.emplace_back(ip, n.payload(), w);
            total += w;
        }
    }

    // no neighbours inside the kernel: the nearest neighbour (the first), so every output point is set
    const auto sum = total.value();
    if (!(sum > 0.)) {
        triplets.assign(1, {ip, neighbours.front().payload(), 1.});
        return;
    }

    for (auto& t : triplets) {
        t.value() /= sum;
    }
}


bool DirectionalKernel::sameAs(const DistanceWeighting& other) const {
    using eckit::types::is_approximately_equal;
    const auto* o = dynamic_cast<const DirectionalKernel*>(&other);
    return (o != nullptr) && profile_.sameAs(o->profile_) && is_approximately_equal(along_, o->along_) &&
           is_approximately_equal(across_, o->across_) && is_approximately_equal(angle_, o->angle_);
}


void DirectionalKernel::json(eckit::JSON& j) const {
    j << type() << "directional";
    j << "kernel" << profile_.name();
    j << "kernel-radius" << along_;
    j << "kernel-radius-across" << across_;
    j << "kernel-angle" << angle_;
}


void DirectionalKernel::print(std::ostream& out) const {
    out << "DirectionalKernel[profile=";
    profile_.print(out);
    out << ",radius=" << along_ << ",radiusAcross=" << across_ << ",angle=" << angle_ << "]";
}


void DirectionalKernel::hash(eckit::MD5& h) const {
    std::ostringstream s;
    s << *this;
    h.add(s.str());
}


static const DistanceWeightingBuilder<DirectionalKernel> __distance("directional");


}  // namespace mir::method::knn::distance
