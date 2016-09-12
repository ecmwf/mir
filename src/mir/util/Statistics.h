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


#ifndef mir_util_Statistics_H
#define mir_util_Statistics_H

#include <cmath>
#include <complex>
#include "mir/util/Angles.h"
#include "mir/util/Compare.h"


namespace mir {
namespace util {
namespace statistics {


/**
 * Counter binary operator functor: missing values
 * @note returns if values are good for comparison (ie. neither is missing)
 */
template< typename T >
struct CountMissingValuesFn : compare::ACompareBinFn<T> {
protected:
    const compare::IsMissingFn isMissing1_;
    const compare::IsMissingFn isMissing2_;
    size_t countMissing1_;
    size_t countMissing2_;
    size_t countDifferencesMissing_;
    size_t countNonMissing_;
    size_t countTotal_;

public:

    CountMissingValuesFn(
            const double& missingValue1=std::numeric_limits<double>::quiet_NaN(),
            const double& missingValue2=std::numeric_limits<double>::quiet_NaN() ) :
        isMissing1_(missingValue1),
        isMissing2_(missingValue2) {
        reset();
    }

    void reset() {
        countMissing1_   = 0;
        countMissing2_   = 0;
        countDifferencesMissing_ = 0;
        countNonMissing_ = 0;
        countTotal_      = 0;
    }

    size_t count()        const { return countNonMissing(); }
    size_t countMissing() const { return countMissing1(); }

    size_t countMissing1()   const { return countMissing1_; }
    size_t countMissing2()   const { return countMissing2_; }
    size_t countDifferencesMissing() const { return countDifferencesMissing_; }
    size_t countNonMissing() const { return countNonMissing_; }
    size_t countTotal()      const { return countTotal_; }

    /// @returns if value is good for comparison (ie. is not missing)
    bool operator()(const T& v) {
        ++countTotal_;
        if (isMissing1_(v)) {
            ++countMissing1_;
            return false;
        }
        ++countNonMissing_;
        return true;
    }

    /// @returns if value is good for comparison (ie. both are not missing)
    bool operator()(const T& v1, const T& v2) {
        ++countTotal_;
        if (!isMissing1_(v1) && !isMissing1_(v2)) {
            ++countNonMissing_;
            return true;
        }
        if (isMissing1_(v1) != isMissing1_(v2)) {
            ++countDifferencesMissing_;
        }
        if (isMissing1_(v1)) { ++countMissing1_; }
        if (isMissing2_(v2)) { ++countMissing2_; }
        return false;
    }

    bool operator+=(const CountMissingValuesFn& other) const {
        countMissing1_           += other.countMissing1_;
        countMissing2_           += other.countMissing2_;
        countDifferencesMissing_ += other.countDifferencesMissing_;
        countNonMissing_         += other.countNonMissing_;
        countTotal_              += other.countTotal_;
        return true;
    }
};


/**
 * Counter unary operator functor: outside range
 */
template< typename T >
struct CountValueOutsideRangeFn {
protected:
    const T lowerLimit_;
    const T upperLimit_;
    size_t count_;

public:

    CountValueOutsideRangeFn(
            const T& lowerLimit=std::numeric_limits<T>::quiet_NaN(),
            const T& upperLimit=std::numeric_limits<T>::quiet_NaN() ) :
        lowerLimit_(lowerLimit),
        upperLimit_(upperLimit) {
        reset();
    }

    void reset() {
        count_ = 0;
    }

    size_t count() const { return count_; }

    bool operator()(const T& v) {
        if (lowerLimit_==lowerLimit_ && v<lowerLimit_) { ++count_; }
        if (upperLimit_==upperLimit_ && v>upperLimit_) { ++count_; }
        return true;
    }

    bool operator+=(const CountValueOutsideRangeFn& other) const {
        count_ += other.count_;
        return true;
    }
};


/**
 * Statistics unary operator functor: min/max range
 * @note: operator+= (the pairwise version) invalidates minIndex() and maxIndex() because they are local to the functor
 */
template< typename T >
struct ScalarValueMinMaxFn {
private:
    T min_;
    T max_;
    size_t minIndex_;
    size_t maxIndex_;
    size_t count_;

public:

    ScalarValueMinMaxFn() { reset(); }

    void reset() {
        min_ = std::numeric_limits<T>::quiet_NaN();
        max_ = std::numeric_limits<T>::quiet_NaN();
        minIndex_ = 0;
        maxIndex_ = 0;
        count_ = 0;
    }

    T min() const { return min_; }
    T max() const { return max_; }
    size_t minIndex() const { return minIndex_; }
    size_t maxIndex() const { return maxIndex_; }

    bool operator()(const T& v) {
        ++count_;
        if (min_!=min_ || min_>v) { min_ = v; minIndex_ = count_; }
        if (max_!=max_ || max_<v) { max_ = v; maxIndex_ = count_; }
        return true;
    }

    bool operator+=(const ScalarValueMinMaxFn& other) {
        if (other.count_) {
            if (min_!=min_ || min_>other.min_) { min_ = other.min_; minIndex_ = 0; }
            if (max_!=max_ || max_<other.max_) { max_ = other.max_; maxIndex_ = 0; }
        }
        return true;
    }
};


/**
 * Statistics unary operator functor: standardized/central moments, up to 4th-order
 * @see Sandia Report SAND2008-6212 "Formulas for Robust, One-Pass Parallel Computation of Covariances and Arbitrary-Order Statistical Moments," 2008 P. Pébay
 * @see http://people.xiph.org/~tterribe/notes/homs.html "Computing Higher-Order Moments Online," 2008 B. Terriberry
 * @note: variance is computed as an unbiased estimator (unbiased sample variance), s^2_{N-1} @see http://mathworld.wolfram.com/Variance.html
 * @note: skewness (γ_1 = μ_3/μ_2^{3/2}) is computed as per @see http://mathworld.wolfram.com/Skewness.html
 * @note: kurtosis (γ_2 = μ_4/μ_2^2 - 3) is computed as kurtosis "excess", @see http://mathworld.wolfram.com/Kurtosis.html
 */
template< typename T >
struct ScalarValueCentralMomentsFn {
private:
    T M1_;
    T M2_;
    T M3_;
    T M4_;
    size_t count_;

public:

    ScalarValueCentralMomentsFn() {
        reset();
    }

    void reset() {
        M1_ = 0;
        M2_ = 0;
        M3_ = 0;
        M4_ = 0;
        count_ = 0;
    }

    T centralMoment1() const { return count_<1? 0 : M1_/T(count_); }
    T centralMoment2() const { return count_<1? 0 : M2_/T(count_); }
    T centralMoment3() const { return count_<1? 0 : M3_/T(count_); }
    T centralMoment4() const { return count_<1? 0 : M4_/T(count_); }

    T mean()       const { return count_<1? std::numeric_limits<T>::quiet_NaN() : M1_; }
    T variance()   const { return count_<2? 0 : (M2_ / T(count_ - 1)); }
    T skewness()   const { return count_<2? 0 : (M3_ * std::sqrt(T(count_))) / std::pow(M2_, 1.5); }
    T kurtosis()   const { return count_<2? 0 : (M4_ * T(count_)) / (M2_*M2_) - 3.; }
    T standardDeviation() const { return std::sqrt(variance()); }
    size_t count() const { return count_; }

    bool operator()(const T& v) {
        const T n1   = T(count_);
        const T n    = T(count_ + 1);
        const T dx   = v - M1_;
        const T dx_n = dx/n;
        M4_ += -4.*dx_n*M3_ + 6.*dx_n*dx_n*M2_ + n1*(n1*n1 - n1 + 1.)*dx_n*dx_n*dx_n*dx;
        M3_ += -3.*dx_n*M2_ + n1*(n1 - 1.)*dx_n*dx_n*dx;
        M2_ += n1*dx_n*dx;
        M1_ += dx_n;

        count_++;
        return true;
    }

    bool operator+=(const ScalarValueCentralMomentsFn& other) {
        if (other.count_) {
            const T n1   = T(count_);
            const T n2   = T(other.count_);
            const T n    = T(count_ + other.count_);
            const T dx   = other.M1_ - M1_;
            const T dx_n = dx/n;
            M4_ += other.M4_
                    + n1*n2*(n1*n1 - n1*n2 + n2*n2)*dx_n*dx_n*dx_n*dx
                    + 6.*(n1*n1*other.M2_ + n2*n2*M2_)*dx_n*dx_n
                    + 4.*(   n1*other.M3_ -    n2*M3_)*dx_n;
            M3_ += other.M3_
                    + n1*n2*(n1 - n2)*dx_n*dx_n*dx
                    + 3.*(n1*other.M2_ - n2*M2_)*dx_n;
            M2_ += other.M2_
                    + n1*n2*dx_n*dx;
            M1_ += n2*dx_n;
        }

        count_ += other.count_;
        return true;
    }
};


/**
 * Statistics unary operator functor: p-norm (p=1, p=2 and p=infinity)
 * @see https://en.wikipedia.org/wiki/Lp_space
 * @see https://en.wikipedia.org/wiki/Minkowski_distance
 */
template< typename T >
struct ScalarValuepNormsFn {
private:
    T normL1_;
    T sumSquares_;
    T normLinfinity_;

public:

    ScalarValuepNormsFn() {
        reset();
    }

    void reset() {
        normL1_        = 0;
        sumSquares_    = 0;
        normLinfinity_ = 0;
    }

    T normL1()        const { return normL1_; }
    T normL2()        const { return std::sqrt(sumSquares_); }
    T normLinfinity() const { return normLinfinity_; }

    bool operator()(const T& v) {
        normL1_       += std::abs(v);
        sumSquares_   += v*v;
        normLinfinity_ = std::max(normLinfinity_, std::abs(v));
        return true;
    }

    bool operator+=(const ScalarValuepNormsFn& other) {
        normL1_       += other.normL1_;
        sumSquares_   += other.sumSquares_;
        normLinfinity_ = std::max(normLinfinity_, other.normLinfinity_);
        return true;
    }
};


/**
 * Statistics unary operator functor: average/std. deviation/variance (suitable for angles in [0°, 360°[)
 * @note: uses scalar statistics of complex numbers, operating on argument and discarding modulus
 */
template< typename T >
struct PolarAngleDegreesCentralMomentsFn : ScalarValueCentralMomentsFn< std::complex<T> > {
private:
    typedef ScalarValueCentralMomentsFn< std::complex<T> > complex;

public:

    PolarAngleDegreesCentralMomentsFn() : complex() {}
    using complex::reset;
    using complex::count;
    using complex::operator+=;

    T mean()     const { return angles::convert_complex_to_degrees<T>(complex::mean()); }
    T variance() const { return angles::convert_complex_to_degrees<T>(complex::variance()); }
    T skewness() const { return angles::convert_complex_to_degrees<T>(complex::skewness()); }  // works, but is it meaningful for circular quantities?
    T kurtosis() const { return angles::convert_complex_to_degrees<T>(complex::kurtosis()); }  // works, but is it meaningful for circular quantities?
    T standardDeviation() const { return std::sqrt(variance()); }

    bool operator()(const T& th) {
        return complex::operator()(angles::convert_degrees_to_complex<T>(th));
    }
};


/**
 * Statistics unary operator functor: composition of above functionality (suitable for scalars)
 */
template< typename T >
struct ScalarValueStatistics : CountMissingValuesFn<T> {
private:
    ScalarValueMinMaxFn<T>         calculateMinMax_;
    ScalarValueCentralMomentsFn<T> calculateCentralMoments_;
    ScalarValuepNormsFn<T>         calculateNorms_;

public:

    ScalarValueStatistics(
            const double& missingValue1=std::numeric_limits<double>::quiet_NaN(),
            const double& missingValue2=std::numeric_limits<double>::quiet_NaN()) :
        CountMissingValuesFn<T>(missingValue1, missingValue2) {
        reset();
    }

    void reset() {
        CountMissingValuesFn<T>::reset();
        calculateMinMax_.reset();
        calculateCentralMoments_.reset();
        calculateNorms_.reset();
    }

    T min()               const { return calculateMinMax_.min(); }
    T max()               const { return calculateMinMax_.max(); }
    size_t minIndex()     const { return calculateMinMax_.minIndex(); }
    size_t maxIndex()     const { return calculateMinMax_.maxIndex(); }

    T mean()              const { return calculateCentralMoments_.mean(); }
    T variance()          const { return calculateCentralMoments_.variance(); }
    T standardDeviation() const { return calculateCentralMoments_.standardDeviation(); }
    T skewness()          const { return calculateCentralMoments_.skewness(); }
    T kurtosis()          const { return calculateCentralMoments_.kurtosis(); }

    T normL1()            const { return calculateNorms_.normL1(); }
    T normL2()            const { return calculateNorms_.normL2(); }
    T normLinfinity()     const { return calculateNorms_.normLinfinity(); }

    bool operator()(const T& v) {
        return CountMissingValuesFn<T>::operator()(v)
                && calculateMinMax_        (v)
                && calculateCentralMoments_(v)
                && calculateNorms_         (v);
    }

    bool operator()(const T& v1, const T& v2) {
        if (CountMissingValuesFn<T>::operator()(v1, v2)) {
            // if value is good for comparison
            const T v = std::abs(v1 - v2);
            return calculateMinMax_        (v)
                && calculateCentralMoments_(v)
                && calculateNorms_         (v);
        }
        return false;
    }

    bool operator+=(const ScalarValueStatistics& other) {
        CountMissingValuesFn<T>::operator+=(other);
        calculateMinMax_         += other.calculateMinMax_;
        calculateCentralMoments_ += other.calculateCentralMoments_;
        calculateNorms_          += other.calculateNorms_;
        return true;
    }
};


/**
 * Statistics unary operator functor: composition of above functionality (suitable for angles in [0°, 360°[)
 */
template< typename T >
struct PolarAngleDegreesStatistics : CountMissingValuesFn<T> {
private:
    ScalarValueMinMaxFn<T>               calculateMinMax_;
    PolarAngleDegreesCentralMomentsFn<T> calculateCentralMoments_;

public:

    PolarAngleDegreesStatistics(
            const double& missingValue1=std::numeric_limits<double>::quiet_NaN(),
            const double& missingValue2=std::numeric_limits<double>::quiet_NaN()) :
        CountMissingValuesFn<T>(missingValue1, missingValue2) {
        calculateCentralMoments_.reset();
    }

    void reset() {
        CountMissingValuesFn<T>::reset();
        calculateMinMax_.reset();
        calculateCentralMoments_.reset();
    }

    T min()               const { return calculateMinMax_.min(); }
    T max()               const { return calculateMinMax_.max(); }
    size_t minIndex()     const { return calculateMinMax_.minIndex(); }
    size_t maxIndex()     const { return calculateMinMax_.maxIndex(); }

    T mean()              const { return calculateCentralMoments_.mean(); }
    T variance()          const { return calculateCentralMoments_.variance(); }
    T standardDeviation() const { return calculateCentralMoments_.standardDeviation(); }

    bool operator()(const T& v) {
        return CountMissingValuesFn<T>::operator()(v)
                && calculateMinMax_        (v)
                && calculateCentralMoments_(v);
    }

    bool operator()(const T& v1, const T& v2) {
        if (CountMissingValuesFn<T>::operator()(v1, v2)) {
            // if value is good for comparison
            const T angleDegreesDifference = std::abs(
                        angles::convert_complex_to_degrees(
                            angles::convert_degrees_to_complex( v1 - v2 ) ));
            return calculateMinMax_        (angleDegreesDifference)
                && calculateCentralMoments_(angleDegreesDifference);
        }
        return false;
    }

    bool operator+=(const PolarAngleDegreesStatistics& other) {
        CountMissingValuesFn<T>::operator+=(other);
        calculateMinMax_         += other.calculateMinMax_;
        calculateCentralMoments_ += other.calculateCentralMoments_;
        return true;
    }
};


}  // namespace statistics
}  // namespace util
}  // namespace mir


#endif
