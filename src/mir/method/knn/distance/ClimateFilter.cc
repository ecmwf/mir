/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/method/knn/distance/ClimateFilter.h"

#include <cmath>
#include <string>

#include "eckit/exception/Exceptions.h"
#include "eckit/utils/MD5.h"

#include "mir/param/MIRParametrisation.h"


namespace mir {
namespace method {
namespace knn {
namespace distance {


ClimateFilter::ClimateFilter(const param::MIRParametrisation& param) {
    distance_ = 1.;
    param.get("distance", distance_);
    ASSERT(distance_ > 0.);

    delta_ = 1000.;
    param.get("climate-filter-delta", delta_);
    ASSERT(delta_ > 0.);

    if (delta_ > distance_) {
        auto str = [](const std::string& option, double value) {
            return "option '" + option + "' = " + std::to_string(value);
        };

        const std::string msg = "ClimateFilter: " + str("distance", distance_) +
                                " should be greater than " + str("climate-filter-delta", delta_);
        eckit::Log::error() << msg << std::endl;
        throw eckit::UserError(msg);
    }
}


void ClimateFilter::operator()(
        size_t ip,
        const Point3& point,
        const std::vector<search::PointSearch::PointValueType>& neighbours,
        std::vector<WeightMatrix::Triplet>& triplets ) const {

    const size_t nbPoints = neighbours.size();
    ASSERT(nbPoints);

    triplets.clear();
    triplets.reserve(nbPoints);

    // calculate neighbour points weights, and their total (for normalisation)
    std::vector<double> weights(nbPoints);
    double sum = 0.;
    for (size_t j = 0; j < nbPoints; ++j) {
        auto r = Point3::distance(point, neighbours[j].point());
        auto h = 0.5 + 0.5 * std::cos(M_PI_2 * (r - 0.5 * distance_ + delta_) / (2. * delta_));
        h = std::max(0., std::min(0.99, h));

        weights[j] = h;
        sum += h;
    }

    ASSERT(sum > 0.);

    // normalise all weights according to the total, and set sparse matrix triplets
    for (size_t i = 0; i < nbPoints; ++i) {
        size_t jp = neighbours[i].payload();
        triplets.emplace_back(WeightMatrix::Triplet(ip, jp, weights[i] / sum));
    }
}


bool ClimateFilter::sameAs(const DistanceWeighting& other) const {
    auto o = dynamic_cast<const ClimateFilter*>(&other);
    return o;
}


void ClimateFilter::print(std::ostream& out) const {
    out << "ClimateFilter[]";
}


void ClimateFilter::hash(eckit::MD5& h) const {
    std::ostringstream s;
    s << *this;
    h.add(s.str());
}


static DistanceWeightingBuilder<ClimateFilter> __distance("climate-filter");


}  // namespace distance
}  // namespace knn
}  // namespace method
}  // namespace mir

