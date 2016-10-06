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


#ifndef mir_action_statistics_detail_AngleStatistics_H
#define mir_action_statistics_detail_AngleStatistics_H

#include <complex>
#include "mir/action/statistics/detail/CountMissingValuesFn.h"
#include "mir/action/statistics/detail/ScalarCentralMomentsFn.h"
#include "mir/action/statistics/detail/ScalarMinMaxFn.h"
#include "mir/method/decompose/PolarAngleToCartesian.h"


namespace mir {
namespace action {
namespace statistics {
namespace detail {


/**
 * Statistics unary operator functor: composition of above functionality (suitable for angles in [0°, 360°[)
 */
template< typename T, int FIELDINFO_COMPONENT >
struct AngleStatistics : CountMissingValuesFn<T> {
private:
    typedef CountMissingValuesFn<T> missing_t;
    typedef method::decompose::PolarAngleToCartesian< FIELDINFO_COMPONENT > decompose_t;

    decompose_t decompose_;
    ScalarMinMaxFn<T> calculateMinMax_;
    ScalarCentralMomentsFn< std::complex<T> > calculateCentralMoments_;

public:

    AngleStatistics(const double& missingValue=std::numeric_limits<double>::quiet_NaN()) {
        reset(missingValue);
    }

    void reset(double missingValue=std::numeric_limits<double>::quiet_NaN()) {
        missing_t::reset(missingValue);
        calculateMinMax_.reset();
        calculateCentralMoments_.reset();
    }

    T min()               const { return calculateMinMax_.min(); }
    T max()               const { return calculateMinMax_.max(); }
    size_t minIndex()     const { return calculateMinMax_.minIndex(); }
    size_t maxIndex()     const { return calculateMinMax_.maxIndex(); }

    T mean()              const { return decompose_.recompose(calculateCentralMoments_.mean()); }
    T variance()          const { return decompose_.recompose(calculateCentralMoments_.variance()); }
//  T skewness()          const { return decompose_.recompose(calculateCentralMoments_.skewness()); }  // works, but is it meaningful for circular quantities?
//  T kurtosis()          const { return decompose_.recompose(calculateCentralMoments_.kurtosis()); }  // works, but is it meaningful for circular quantities?
    T standardDeviation() const { return std::sqrt(std::abs(variance()));; }

    bool operator()(const T& v) {
        return missing_t::operator()(v)
                && calculateMinMax_        (decompose_.normalize(v))
                && calculateCentralMoments_(decompose_.decompose(v));
    }

    bool operator()(const T& v1, const T& v2) {
        if (missing_t::operator()(v1, v2)) {
            // if value is good for comparison
            const T v = decompose_.normalize(v1 - v2);
            return calculateMinMax_        (decompose_.normalize(v))
                && calculateCentralMoments_(decompose_.decompose(v));
        }
        return false;
    }

    bool operator+=(const AngleStatistics& other) {
        missing_t::operator+=(other);
        calculateMinMax_         += other.calculateMinMax_;
        calculateCentralMoments_ += other.calculateCentralMoments_;
        return true;
    }
};


}  // namespace detail
}  // namespace statistics
}  // namespace action
}  // namespace mir


#endif
