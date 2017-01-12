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
    IsMissingFn(double missingValue=std::numeric_limits<double>::quiet_NaN()) {
        reset(missingValue);
    }
    void reset(double missingValue=std::numeric_limits<double>::quiet_NaN()) {
        missingValue_ = missingValue;
        missingValueDefined_ = (missingValue==missingValue);
    }
    bool operator()(const double& v) const {
        return (missingValueDefined_ && (missingValue_==v));
    }
    double missingValue_;
    bool missingValueDefined_;
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
        return eckit::types::is_approximately_equal(v, ref_);
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
struct is_approximately_greater_or_equalFn : ACompareFn<T> {
    is_approximately_greater_or_equalFn(T ref) : ref_(ref) {}
    bool operator()(const T& v) const {
#if 0
        // FIXME: What should it be? give me a resource
        return ( a >= b )? || (fabs(a-b) < 1e-10);
#endif
        return eckit::types::is_approximately_greater_or_equal(v, ref_);
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
        return eckit::types::is_approximately_lesser_or_equal(v, ref_);
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


/// Utility comparators
extern const IsApproximatelyEqualFn< double > is_approx_zero;
extern const IsApproximatelyEqualFn< double > is_approx_one;


}  // namespace compare
}  // namespace util
}  // namespace mir


#endif
