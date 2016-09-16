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

#include "mir/util/Statistics.h"


namespace mir {
namespace action {
namespace statistics {
namespace detail {


/**
 * Statistics unary operator functor: composition of above functionality (suitable for angles in [0°, 360°[)
 */
template< typename T >
struct AngleStatistics : util::statistics::CountMissingValuesFn<T> {
private:
    util::statistics::ScalarMinMaxFn<T>        calculateMinMax_;
    util::statistics::AngleCentralMomentsFn<T> calculateCentralMoments_;
    typedef util::statistics::CountMissingValuesFn<T> missing_t;
    //NOTE: not using difference mode for the moment (2nd argument to missing_t)

public:

    AngleStatistics(const double& missingValue=std::numeric_limits<double>::quiet_NaN()) {
        reset(missingValue);
    }

    void reset(
            double missingValue=std::numeric_limits<double>::quiet_NaN(),
            bool degrees=true,
            bool symmetric=false ) {
        missing_t::reset(missingValue);
        calculateMinMax_.reset();
        calculateCentralMoments_.reset(degrees, symmetric);
    }

    T min()               const { return calculateMinMax_.min(); }
    T max()               const { return calculateMinMax_.max(); }
    size_t minIndex()     const { return calculateMinMax_.minIndex(); }
    size_t maxIndex()     const { return calculateMinMax_.maxIndex(); }

    T mean()              const { return calculateCentralMoments_.mean(); }
    T variance()          const { return calculateCentralMoments_.variance(); }
//  T skewness()          const { return calculateCentralMoments_.skewness(); }  // works, but is it meaningful for circular quantities?
//  T kurtosis()          const { return calculateCentralMoments_.kurtosis(); }  // works, but is it meaningful for circular quantities?
    T standardDeviation() const { return calculateCentralMoments_.standardDeviation(); }

    bool operator()(const T& v) {
        return missing_t::operator()(v)
                && calculateMinMax_        (v)
                && calculateCentralMoments_(v);
    }

    bool operator()(const T& v1, const T& v2) {
        if (missing_t::operator()(v1, v2)) {
            // if value is good for comparison
            const T v = std::abs(v1 - v2);
            return calculateMinMax_        (v)
                && calculateCentralMoments_(v);
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
