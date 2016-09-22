/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file   MIRStatistics.h
/// @author Baudouin Raoult
/// @author Tiago Quintino
/// @date   April 2016

#ifndef util_MIRStatistics_H
#define util_MIRStatistics_H

#include <iosfwd>

#include "eckit/log/Statistics.h"
#include "mir/caching/InMemoryCacheStatistics.h"

namespace eckit {
class Stream;
}

namespace mir {
namespace util {

//----------------------------------------------------------------------------------------------------------------------

class MIRStatistics : public eckit::Statistics {
public:
    MIRStatistics() ;
    MIRStatistics(eckit::Stream &) ;

    MIRStatistics &operator+=(const MIRStatistics &rhs) ;
    MIRStatistics &operator/=(size_t) ;

    InMemoryCacheStatistics bitmapCache_;
    InMemoryCacheStatistics areaCroppingCache_;
    InMemoryCacheStatistics transHandleCache_;
    InMemoryCacheStatistics matrixCache_;
    InMemoryCacheStatistics meshCache_;

    eckit::Timing cropTiming_;
    eckit::Timing frameTiming_;
    eckit::Timing bitmapTiming_;

    eckit::Timing coefficientTiming_;
    eckit::Timing sh2gridTiming_;
    eckit::Timing grid2gridTiming_;
    eckit::Timing vod2uvTiming_;
    eckit::Timing computeMatrixTiming_;
    eckit::Timing matrixTiming_;
    eckit::Timing loadCoeffTiming_;
    eckit::Timing createCoeffTiming_;
    eckit::Timing calcTiming_;



    void report(std::ostream &out, const char *indent = "") const;

    void encode(eckit::Stream &) const;

    friend eckit::Stream &operator<<(eckit::Stream &s, const MIRStatistics &x) {
        x.encode(s);
        return s;
    }
};

//----------------------------------------------------------------------------------------------------------------------

} // namespace util
} // namespace mir

#endif
