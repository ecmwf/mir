// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/method/knn/distance/RadialKernel.h"

#include <ostream>
#include <sstream>

#include "eckit/log/JSON.h"
#include "eckit/types/FloatCompare.h"
#include "eckit/utils/MD5.h"

#include "mir/util/CompensatedSum.h"
#include "mir/util/Exceptions.h"


namespace mir::method::knn::distance {


RadialKernel::RadialKernel(const param::MIRParametrisation& param, const std::string& profile) :
    RadialKernel(param, profile, kernel_radius(param)) {}


RadialKernel::RadialKernel(const param::MIRParametrisation& param, const std::string& profile, double radius) :
    profile_(profile), radius_(radius) {
    ASSERT(radius_ > 0.);
}


void RadialKernel::operator()(size_t ip, const Point3& point,
                              const std::vector<search::PointSearch::PointValueType>& neighbours,
                              std::vector<WeightMatrix::Triplet>& triplets) const {
    ASSERT(!neighbours.empty());

    triplets.clear();
    triplets.reserve(neighbours.size());

    // calculate neighbour points weights, and their total (for normalisation)
    util::CompensatedSum<double> total;  // the weights add up to one, even for many neighbours
    for (const auto& n : neighbours) {
        const auto w = profile_.at_distance2(Point3::distance2(point, n.point()), radius_);
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

    // normalise all weights according to the total weights sum
    for (auto& t : triplets) {
        t.value() /= sum;
    }
}


bool RadialKernel::sameAs(const DistanceWeighting& other) const {
    const auto* o = dynamic_cast<const RadialKernel*>(&other);
    return (o != nullptr) && profile_.sameAs(o->profile_) && eckit::types::is_approximately_equal(radius_, o->radius_);
}


void RadialKernel::json(eckit::JSON& j) const {
    j << type() << profile_.name();
    j << "kernel-radius" << radius_;
}


void RadialKernel::print(std::ostream& out) const {
    out << "RadialKernel[profile=";
    profile_.print(out);
    out << ",radius=" << radius_ << "]";
}


void RadialKernel::hash(eckit::MD5& h) const {
    std::ostringstream s;
    s << *this;
    h.add(s.str());
}


namespace {


template <const char* NAME>
struct RadialKernelT final : RadialKernel {
    explicit RadialKernelT(const param::MIRParametrisation& param) : RadialKernel(param, NAME) {}
};


constexpr char TOP_HAT[]      = "top-hat";
constexpr char HANN[]         = "hann";
constexpr char EPANECHNIKOV[] = "epanechnikov";


const DistanceWeightingBuilder<RadialKernelT<TOP_HAT>> __distance1(TOP_HAT);
const DistanceWeightingBuilder<RadialKernelT<HANN>> __distance2(HANN);
const DistanceWeightingBuilder<RadialKernelT<EPANECHNIKOV>> __distance3(EPANECHNIKOV);


}  // namespace


}  // namespace mir::method::knn::distance
