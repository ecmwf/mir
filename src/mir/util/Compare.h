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


#ifndef mir_util_Compare_h
#define mir_util_Compare_h

#include <limits>


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


}  // namespace compare
}  // namespace util
}  // namespace mir


#endif
