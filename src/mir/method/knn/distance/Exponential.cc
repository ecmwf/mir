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


#include <cmath>
#include <memory>
#include <sstream>

#include "eckit/types/FloatCompare.h"
#include "eckit/utils/MD5.h"

#include "mir/method/knn/distance/DistanceWeighting.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/Exceptions.h"


namespace mir {
namespace method {
namespace knn {
namespace distance {


namespace {


using eckit::types::is_approximately_equal;
using eckit::types::is_strictly_greater;


using calculate_weights_t = double (*)(const Point3& point,
                                       const std::vector<search::PointSearch::PointValueType>& neighbours,
                                       double tolerance2, std::vector<double>& weights);


using calculate_triplets_t = void (*)(size_t ip, const std::vector<search::PointSearch::PointValueType>& neighbours,
                                      const std::vector<double>& weights, double sum,
                                      std::vector<WeightMatrix::Triplet>& triplets);


double weights_no_tolerance(const Point3& point, const std::vector<search::PointSearch::PointValueType>& neighbours,
                            double /*tolerance2*/, std::vector<double>& weights) {
    double sum = 0.;
    for (const auto& neighbour : neighbours) {
        auto w = is_approximately_equal(Point3::distance2(point, neighbour.point()), 0.) ? 1. : 0.;
        weights.emplace_back(w);
        sum += w;
    }
    return sum;
}


double weights_exponential(const Point3& point, const std::vector<search::PointSearch::PointValueType>& neighbours,
                           double tolerance2, std::vector<double>& weights) {
    // assumes neighbours are sorted by (increasing) distance
    auto d2 = Point3::distance2(point, neighbours.front().point());
    if (d2 < tolerance2) {
        weights = {1.};
        return 1.;
    }

    double sum = 0.;
    for (const auto& neighbour : neighbours) {
        auto w = std::exp(-Point3::distance2(point, neighbour.point()) / tolerance2);
        weights.emplace_back(w);
        sum += w;
    }
    return sum;
}


double weights_reciprocal(const Point3& point, const std::vector<search::PointSearch::PointValueType>& neighbours,
                          double tolerance2, std::vector<double>& weights) {
    // assumes neighbours are sorted by (increasing) distance
    auto d2 = Point3::distance2(point, neighbours.front().point());
    if (d2 < tolerance2) {
        weights = {1.};
        return 1.;
    }

    double sum = 0.;
    for (const auto& neighbour : neighbours) {
        auto w = 1. / Point3::distance2(point, neighbour.point());
        weights.emplace_back(w);
        sum += w;
    }
    return sum;
}


void triplets_mean(size_t ip, const std::vector<search::PointSearch::PointValueType>& neighbours,
                   const std::vector<double>& weights, double sum, std::vector<WeightMatrix::Triplet>& triplets) {
    if (!is_strictly_greater(sum, 0.)) {
        return;
    }

    auto nbPoints = weights.size();
    ASSERT(nbPoints <= neighbours.size());

    for (size_t j = 0; j < nbPoints; ++j) {
        if (weights[j] > 0.) {
            auto jp = neighbours[j].payload();
            auto w  = weights[j] / sum;
            if (is_strictly_greater(w, 0.)) {
                triplets.emplace_back(WeightMatrix::Triplet(ip, jp, w));
            }
        }
    }
}


void triplets_sum(size_t ip, const std::vector<search::PointSearch::PointValueType>& neighbours,
                  const std::vector<double>& weights, double /*sum*/, std::vector<WeightMatrix::Triplet>& triplets) {
    auto nbPoints = weights.size();
    ASSERT(nbPoints <= neighbours.size());

    for (size_t j = 0; j < nbPoints; ++j) {
        if (weights[j] > 0.) {
            auto jp = neighbours[j].payload();
            auto w  = weights[j];
            if (is_strictly_greater(w, 0.)) {
                triplets.emplace_back(WeightMatrix::Triplet(ip, jp, w));
            }
        }
    }
}


double get_tolerance(const param::MIRParametrisation& param) {
    double tolerance = 0.;
    param.get("distance-weighting-tolerance", tolerance);
    ASSERT(tolerance >= 0.);
    return tolerance;
}


}  // namespace


struct Exponential : DistanceWeighting {
    Exponential(const param::MIRParametrisation& param, calculate_weights_t calculateWeights,
                calculate_triplets_t calculateTriplets) :
        tolerance_(get_tolerance(param)),
        tolerance2_(is_approximately_equal(tolerance_ * tolerance_, 0.) ? 0. : tolerance_ * tolerance_),
        weights_(is_approximately_equal(tolerance_, 0.) ? weights_no_tolerance : calculateWeights),
        triplets_(calculateTriplets) {}

    void operator()(size_t ip, const Point3& point, const std::vector<search::PointSearch::PointValueType>& neighbours,
                    std::vector<WeightMatrix::Triplet>& triplets) const override {
        ASSERT(!neighbours.empty());

        std::vector<double> weights;
        weights.reserve(neighbours.size());
        auto sum = weights_(point, neighbours, tolerance2_, weights);

        triplets.clear();
        triplets.reserve(neighbours.size());
        triplets_(ip, neighbours, weights, sum, triplets);

        size_t nbPoints = weights.size();
        ASSERT(nbPoints == neighbours.size());

        for (size_t j = 0; j < nbPoints; ++j) {
            if (weights[j] > 0.) {
                size_t jp   = neighbours[j].payload();
                auto weight = weights[j];
                if (is_strictly_greater(weight, 0.)) {
                    triplets.emplace_back(WeightMatrix::Triplet(ip, jp, weight));
                }
            }
        }
    }

    double tolerance() const { return tolerance_; }

    void hash(eckit::MD5& h) const override {
        std::ostringstream s;
        s << *this;
        h.add(s.str());
    }

private:
    const double tolerance_;
    const double tolerance2_;
    const calculate_weights_t weights_;
    const calculate_triplets_t triplets_;
};


/**
 * Calculates the mean (the sum divided by the total) of the values weighted, or multiplied, by the following:
 * - if tolerance is not zero: exp(−distance2/tolerance2)
 * - if tolerance is zero: 1. if the point is on the target point, 0. otherwise (number of input points matching target
 * points)
 */
struct ExponentialMean final : Exponential {
    explicit ExponentialMean(const param::MIRParametrisation& param) :
        Exponential(param, weights_exponential, triplets_mean) {}

private:
    void print(std::ostream& out) const override { out << "ExponentialMean[tolerance=" << tolerance() << "]"; }

    bool sameAs(const DistanceWeighting& other) const override {
        const auto* o = dynamic_cast<const ExponentialMean*>(&other);
        return (o != nullptr) && is_approximately_equal(tolerance(), o->tolerance());
    }
};


/**
 * Calculates the sum of the values weighted, or multiplied, by the following:
 * - if tolerance is not zero: exp(−distance2/tolerance2)
 * - if tolerance is zero: 1. if the point is on the target point, 0. otherwise (number of input points matching target
 * points)
 */
struct ExponentialSum final : Exponential {
    explicit ExponentialSum(const param::MIRParametrisation& param) :
        Exponential(param, weights_exponential, triplets_sum) {}

    void print(std::ostream& out) const override { out << "ExponentialSum[tolerance=" << tolerance() << "]"; }

    bool sameAs(const DistanceWeighting& other) const override {
        const auto* o = dynamic_cast<const ExponentialSum*>(&other);
        return (o != nullptr) && is_approximately_equal(tolerance(), o->tolerance());
    }
};


/**
 * Calculates the mean of the values weighted, or multiplied, by the (square of the) inverse of their distance from the
 * target point.  If a source point lies exactly on the target point then its value is used directly and the rest of the
 * values discarded.
 */
struct Reciprocal final : Exponential {
    explicit Reciprocal(const param::MIRParametrisation& param) :
        Exponential(param, weights_reciprocal, triplets_mean) {}

    void print(std::ostream& out) const override { out << "Reciprocal[tolerance=" << tolerance() << "]"; }

    bool sameAs(const DistanceWeighting& other) const override {
        const auto* o = dynamic_cast<const Reciprocal*>(&other);
        return (o != nullptr) && is_approximately_equal(tolerance(), o->tolerance());
    }
};


static const DistanceWeightingBuilder<ExponentialMean> __distance_1("exponential-mean");
static const DistanceWeightingBuilder<ExponentialSum> __distance_2("exponential-sum");
static const DistanceWeightingBuilder<Reciprocal> __distance_3("reciprocal");


}  // namespace distance
}  // namespace knn
}  // namespace method
}  // namespace mir
