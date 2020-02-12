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


#ifndef mir_util_MIRStatistics_h
#define mir_util_MIRStatistics_h

#include <iosfwd>

#include "eckit/log/Statistics.h"

#include "mir/caching/InMemoryCacheStatistics.h"


namespace eckit {
class Stream;
}


namespace mir {
namespace util {


class MIRStatistics : public eckit::Statistics {
public:
    MIRStatistics();
    MIRStatistics(eckit::Stream&);

    MIRStatistics(const MIRStatistics&) = delete;
    MIRStatistics& operator=(const MIRStatistics&) = delete;

    MIRStatistics& operator+=(const MIRStatistics&);
    MIRStatistics& operator/=(size_t);

    using AutoTiming = eckit::AutoTiming;
    using Timing     = eckit::Timing;

    caching::InMemoryCacheStatistics bitmapCache_;
    caching::InMemoryCacheStatistics areaCroppingCache_;
    caching::InMemoryCacheStatistics transHandleCache_;
    caching::InMemoryCacheStatistics matrixCache_;
    caching::InMemoryCacheStatistics meshCache_;

    AutoTiming cropTimer() { return {timer_, cropTiming_}; }
    AutoTiming frameTimer() { return {timer_, frameTiming_}; }
    AutoTiming globaliseTimer() { return {timer_, globaliseTiming_}; }
    AutoTiming bitmapTimer() { return {timer_, bitmapTiming_}; }
    AutoTiming coefficientTimer() { return {timer_, coefficientTiming_}; }
    AutoTiming sh2gridTimer() { return {timer_, sh2gridTiming_}; }
    AutoTiming grid2gridTimer() { return {timer_, grid2gridTiming_}; }
    AutoTiming vod2uvTimer() { return {timer_, vod2uvTiming_}; }
    AutoTiming computeMatrixTimer() { return {timer_, computeMatrixTiming_}; }
    AutoTiming matrixTimer() { return {timer_, matrixTiming_}; }
    AutoTiming loadCoeffTimer() { return {timer_, loadCoeffTiming_}; }
    AutoTiming createCoeffTimer() { return {timer_, createCoeffTiming_}; }
    AutoTiming calcTimer() { return {timer_, calcTiming_}; }
    AutoTiming saveTimer() { return {timer_, saveTiming_}; }
    AutoTiming gribEncodingTimer() { return {timer_, gribEncodingTiming_}; }

    Timing& gribEncodingTiming() { return gribEncodingTiming_; }

    void report(std::ostream&, const char* indent = "") const;
    void csvHeader(std::ostream&) const;
    void csvRow(std::ostream&) const;
    void encode(eckit::Stream&) const;

private:
    Timing cropTiming_;
    Timing frameTiming_;
    Timing globaliseTiming_;
    Timing bitmapTiming_;
    Timing coefficientTiming_;
    Timing sh2gridTiming_;
    Timing grid2gridTiming_;
    Timing vod2uvTiming_;
    Timing computeMatrixTiming_;
    Timing matrixTiming_;
    Timing loadCoeffTiming_;
    Timing createCoeffTiming_;
    Timing calcTiming_;
    Timing saveTiming_;
    Timing gribEncodingTiming_;

    friend eckit::Stream& operator<<(eckit::Stream& s, const MIRStatistics& x) {
        x.encode(s);
        return s;
    }
};


}  // namespace util
}  // namespace mir


#endif
