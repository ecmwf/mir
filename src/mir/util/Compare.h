/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Tiago Quintino
/// @author Pedro Maciel
/// @date May 2015


#ifndef mir_util_Compare_H
#define mir_util_Compare_H

#include <cmath>
#include <cstddef>
#include <vector>
#include "eckit/exception/Exceptions.h"
#include "eckit/types/FloatCompare.h"


namespace mir {
namespace util {
namespace compare {


/// Compare values parent type (abstract comparison unary operator functor)
template< typename T >
struct ACompareFn {
    virtual ~ACompareFn() {}
    virtual bool operator()(const T&) const = 0;
};


/// Compare values parent type (abstract comparison binary operator functor)
template< typename T >
struct ACompareBinFn {
    virtual ~ACompareBinFn() {}
    virtual bool operator()(const T&, const T&) = 0;
};


/// Compare to missing values
struct IsMissingFn : ACompareFn<double> {
    IsMissingFn(double missingValue) :
        missingValue_(missingValue),
        missingValueDefined_(missingValue==missingValue) {}
    bool operator()(const double& v) const {
        return (missingValueDefined_ && (missingValue_==v));
    }
    const double missingValue_;
    const bool missingValueDefined_;
};


/// Avoid comparing
template< typename T >
struct IsAnythingFn : ACompareFn<T> {
    bool operator()(const T&) const {
        return true;
    }
};


/// Compare values equality
template< typename T >
struct IsEqualFn : ACompareFn<T> {
    IsEqualFn(T ref) : ref_(ref) {}
    bool operator()(const T& v) const {
        return (v == ref_);
    }
    const T ref_;
};


/// Compare values equality, approximately
template< typename T >
struct IsApproximatelyEqualFn : ACompareFn<T> {
    IsApproximatelyEqualFn(T ref) : ref_(ref) {}
    bool operator()(const T& v) const {
        return eckit::FloatCompare<T>::isApproximatelyEqual(v, ref_);
    }
    const T ref_;
};


/// Compare values inequality, "is greater or equal to"
template< typename T >
struct IsGreaterOrEqualFn : ACompareFn<T> {
    IsGreaterOrEqualFn(T ref) : ref_(ref) {}
    bool operator()(const T& v) const {
        return (v >= ref_);
    }
    const T ref_;
};


/// Compare values inequality, "is greater or approximately equal to"
template< typename T >
struct IsApproximatelyGreaterOrEqualFn : ACompareFn<T> {
    IsApproximatelyGreaterOrEqualFn(T ref) : ref_(ref) {}
    bool operator()(const T& v) const {
#if 0
        // FIXME: What should it be? give me a resource
        return ( a >= b )? || (fabs(a-b) < 1e-10);
#endif
        return (v >= ref_) || eckit::FloatCompare<T>::isApproximatelyEqual(v, ref_);
    }
    const T ref_;
};


/// Compare values inequality, "is less than or equal to"
template< typename T >
struct IsLessThanOrEqualFn : ACompareFn<T> {
    IsLessThanOrEqualFn(T ref) : ref_(ref) {}
    bool operator()(const T& v) const {
        return (v <= ref_);
    }
    const T ref_;
};


/// Compare values inequality, "is less than or approximately equal to"
template< typename T >
struct IsApproximatelyLessThanOrEqualFn : ACompareFn<T> {
    IsApproximatelyLessThanOrEqualFn(T ref) : ref_(ref) {}
    bool operator()(const T& v) const {
        return (v <= ref_) || eckit::FloatCompare<T>::isApproximatelyEqual(v, ref_);
    }
    const T ref_;
};


/// Compare if in mask (vector indices)
struct IsMaskedFn : ACompareFn< size_t > {
    IsMaskedFn(const std::vector< bool >& mask) : mask_(mask) {}
    bool operator()(const size_t& i) const {
        // TODO: remove when properly debugged
        ASSERT(i<mask_.size());
        return mask_[i];
    }
    const std::vector< bool >& mask_;
};


/// Compare if not in mask (vector indices)
struct IsNotMaskedFn : ACompareFn< size_t > {
    IsNotMaskedFn(const std::vector< bool >& mask) : mask_(mask) {}
    bool operator()(const size_t& i) const {
        // TODO: remove when properly debugged
        ASSERT(i<mask_.size());
        return !mask_[i];
    }
    const std::vector< bool >& mask_;
};


/// Compare two values (possibly missing) with absolute tolerance
template< typename T >
struct CompareValuesAbsoluteToleranceFn : ACompareBinFn<T> {
    CompareValuesAbsoluteToleranceFn( T epsilon,
            const double& missingValue1=std::numeric_limits<double>::quiet_NaN(),
            const double& missingValue2=std::numeric_limits<double>::quiet_NaN() ) :
        miss1_(missingValue1),
        miss2_(missingValue2),
        eps_(epsilon) {
        ASSERT(eps_>=0);
    }
    bool operator()(const T& v1, const T& v2) {
        if (miss1_(v1) || miss2_(v2))
            return (miss1_(v1) && miss2_(v2));
        return eckit::FloatCompare<T>::isApproximatelyEqual(v1, v2, eps_, 64);  // 64 is chosen so ULPs comparisons don't matter
    }
    IsMissingFn miss1_, miss2_;
    const T eps_;
};


/// Compare two values (possibly missing) with relative tolerance
template< typename T >
struct CompareValuesRelativeToleranceFn : ACompareBinFn<T> {
    CompareValuesRelativeToleranceFn( T epsilon,
            const double& missingValue1=std::numeric_limits<double>::quiet_NaN(),
            const double& missingValue2=std::numeric_limits<double>::quiet_NaN() ) :
        miss1_(missingValue1),
        miss2_(missingValue2),
        eps_(epsilon) {
        ASSERT(eps_>=0.);
    }
    bool operator()(const T& v1, const T& v2) {
        if (miss1_(v1) || miss2_(v2))
            return (miss1_(v1) && miss2_(v2));
        const T dx = std::abs(v1 - v2);
        const T x = std::max(std::numeric_limits<T>::epsilon(), std::max(std::abs(v1), std::abs(v2)));
        return (dx/x <= eps_);
    }
    IsMissingFn miss1_, miss2_;
    const T eps_;
};


/// Utility comparators
extern const IsApproximatelyEqualFn< double > is_approx_zero;
extern const IsApproximatelyEqualFn< double > is_approx_one;


}  // namespace compare
}  // namespace util
}  // namespace mir


#endif
