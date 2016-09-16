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


#ifndef mir_action_statistics_detail_AngleCentralMomentsFn_h
#define mir_action_statistics_detail_AngleCentralMomentsFn_h

#include <complex>
#include "mir/action/statistics/detail/ScalarCentralMomentsFn.h"
#include "mir/util/Angles.h"


namespace mir {
namespace action {
namespace statistics {
namespace detail {


/**
 * Statistics unary operator functor: average/std. deviation/variance (suitable for angles in [0°, 360°[)
 * @note: uses scalar statistics of complex numbers, operating on argument and discarding modulus
 */
template< typename T >
struct AngleCentralMomentsFn : ScalarCentralMomentsFn< std::complex<T> > {
private:
    typedef ScalarCentralMomentsFn< std::complex<T> > complex_t;
    bool degrees_;
    bool symmetric_;

public:

    AngleCentralMomentsFn() : complex_t() {}

    void reset(bool degrees=true, bool symmetric=false) {
        complex_t::reset();
        degrees_   = degrees;
        symmetric_ = symmetric;
    }

    using complex_t::count;
    using complex_t::operator+=;

    T mean()     const { return util::angles::convert_complex_to_degrees<T>(complex_t::mean()); }
    T variance() const { return util::angles::convert_complex_to_degrees<T>(complex_t::variance()); }
    T skewness() const { return util::angles::convert_complex_to_degrees<T>(complex_t::skewness()); }  // works, but is it meaningful for circular quantities?
    T kurtosis() const { return util::angles::convert_complex_to_degrees<T>(complex_t::kurtosis()); }  // works, but is it meaningful for circular quantities?
    T standardDeviation() const { return std::sqrt(std::abs(variance())); }

    bool operator()(const T& th) {
        using namespace util::angles;
        T th_normalized = degrees_? (symmetric_? between_m180_and_p180 : between_0_and_360)(th)
                                  : (symmetric_? between_mPI_and_pPI   : between_0_and_2PI)(th);
        return complex_t::operator()(degrees_? convert_degrees_to_complex<T>(th_normalized)
                                             : std::polar<T>(1, th_normalized) );
    }
};


}  // namespace detail
}  // namespace statistics
}  // namespace action
}  // namespace mir


#endif
