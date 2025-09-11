/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/method/knn/distance/ClimateFilter.h"

#include <cmath>
#include <string>

#include "eckit/log/JSON.h"
#include "eckit/types/FloatCompare.h"
#include "eckit/utils/MD5.h"

#include "mir/param/DefaultParametrisation.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"


namespace mir::method::knn::distance {


ClimateFilter::ClimateFilter(const param::MIRParametrisation& param) {
    auto distance = param::DefaultParametrisation::instance().get_value<double>("distance", param);
    ASSERT(distance > 0.);

    delta_ = 1000.;
    param.get("climate-filter-delta", delta_);
    ASSERT(delta_ > 0.);

    if (delta_ > distance) {
        auto str = [](const std::string& option, double value) {
            return "option '" + option + "' = " + std::to_string(value);
        };

        const std::string msg = "ClimateFilter: " + str("distance", distance) + " should be greater than " +
                                str("climate-filter-delta", delta_);
        Log::error() << msg << std::endl;
        throw exception::UserError(msg);
    }

    halfDelta_ = distance / 2.;
}


void ClimateFilter::operator()(size_t ip, const Point3& point,
                               const std::vector<search::PointSearch::PointValueType>& neighbours,
                               std::vector<WeightMatrix::Triplet>& triplets) const {

    const size_t nbPoints = neighbours.size();
    ASSERT(nbPoints);

    triplets.clear();
    triplets.reserve(nbPoints);

    // calculate neighbour points weights, and their total (for normalisation)
    std::vector<double> weights(nbPoints);
    double sum = 0.;
    for (size_t j = 0; j < nbPoints; ++j) {
        auto r = Point3::distance(point, neighbours[j].point());
        auto h = r < halfDelta_ - delta_   ? 1.
                 : halfDelta_ + delta_ < r ? 0.
                                           : 0.5 + 0.5 * std::cos(M_PI_2 * (r - halfDelta_ + delta_) / delta_);
        // h = std::max(0., std::min(0.99, h));

        weights[j] = h;
        sum += h;
    }

    ASSERT(sum > 0.);

    // normalise all weights according to the total, and set sparse matrix triplets
    for (size_t j = 0; j < nbPoints; ++j) {
        size_t jp = neighbours[j].payload();
        triplets.emplace_back(ip, jp, weights[j] / sum);
    }
}


bool ClimateFilter::sameAs(const DistanceWeighting& other) const {
    const auto* o = dynamic_cast<const ClimateFilter*>(&other);
    return (o != nullptr) && eckit::types::is_approximately_equal(halfDelta_, o->halfDelta_) &&
           eckit::types::is_approximately_equal(delta_, o->delta_);
}


void ClimateFilter::json(eckit::JSON& j) const {
    j << type() << "climate-filter";
    j << "climate-filter-delta" << delta_;
}


void ClimateFilter::print(std::ostream& out) const {
    out << "ClimateFilter[halfDelta=" << halfDelta_ << ",delta=" << delta_ << "]";
}


void ClimateFilter::hash(eckit::MD5& h) const {
    h.add("climate-filter");
    h.add(halfDelta_);
    h.add(delta_);
}


static const DistanceWeightingBuilder<ClimateFilter> __distance("climate-filter");


}  // namespace mir::method::knn::distance
