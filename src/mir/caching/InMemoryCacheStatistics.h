/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file   InMemoryCacheStatistics.h
/// @author Baudouin Raoult
/// @author Tiago Quintino
/// @date   April 2016

#ifndef util_InMemoryCacheStatistics_H
#define util_InMemoryCacheStatistics_H

#include <iosfwd>

#include "eckit/log/Statistics.h"

namespace eckit {
class Stream;
}

namespace mir {

//----------------------------------------------------------------------------------------------------------------------

class InMemoryCacheStatistics : public eckit::Statistics {
public:
    InMemoryCacheStatistics() ;
    InMemoryCacheStatistics(eckit::Stream &) ;

    InMemoryCacheStatistics &operator+=(const InMemoryCacheStatistics &rhs) ;
    InMemoryCacheStatistics &operator/=(size_t) ;


    size_t accesses_;
    size_t evictions_;
    size_t insertions_;
    double oldest_;
    double youngest_;
    size_t capacity_;
    size_t unique_;

    void report(const char *title, std::ostream &out, const char *indent = "") const;

    void encode(eckit::Stream &) const;

    friend eckit::Stream &operator<<(eckit::Stream &s, const InMemoryCacheStatistics &x) {
        x.encode(s);
        return s;
    }
};

//----------------------------------------------------------------------------------------------------------------------

} // namespace pgen

#endif
