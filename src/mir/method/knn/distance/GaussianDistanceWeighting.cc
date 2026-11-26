// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/method/knn/distance/GaussianDistanceWeighting.h"

#include <ostream>

#include "eckit/log/JSON.h"


namespace mir::method::knn::distance {


GaussianDistanceWeighting::GaussianDistanceWeighting(const param::MIRParametrisation& param) :
    RadialKernel(param, "gaussian", kernel_radius(param, "distance-weighting-gaussian-stddev")) {}


bool GaussianDistanceWeighting::sameAs(const DistanceWeighting& other) const {
    return dynamic_cast<const GaussianDistanceWeighting*>(&other) != nullptr && RadialKernel::sameAs(other);
}


void GaussianDistanceWeighting::json(eckit::JSON& j) const {
    j << type() << "gaussian";
    j << "distance-weighting-gaussian-stddev" << radius();
}


void GaussianDistanceWeighting::print(std::ostream& out) const {
    out << "GaussianDistanceWeighting[stddev=" << radius() << "]";
}


static const DistanceWeightingBuilder<GaussianDistanceWeighting> __distance("gaussian");


}  // namespace mir::method::knn::distance
