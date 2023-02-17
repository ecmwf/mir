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


#pragma once

#include <iosfwd>
#include <map>

#include "eckit/log/Statistics.h"

#include "mir/caching/InMemoryCache.h"
#include "mir/caching/InMemoryCacheStatistics.h"


namespace eckit {
class JSON;
class Stream;
}  // namespace eckit


namespace mir::util {


class MIRStatistics : public eckit::Statistics {
public:
    // -- Types

    using AutoTiming = eckit::AutoTiming;
    using Timing     = eckit::Timing;

    // -- Exceptions
    // None

    // -- Constructors

    MIRStatistics();
    MIRStatistics(eckit::Stream&);
    MIRStatistics(const MIRStatistics&) = default;

    // -- Destructor

    virtual ~MIRStatistics() = default;

    // -- Convertors
    // None

    // -- Operators

    MIRStatistics& operator=(const MIRStatistics&) = default;
    MIRStatistics& operator+=(const MIRStatistics&);
    MIRStatistics& operator/=(size_t);

    // -- Members
    // None

    // -- Methods

    template <typename T>
    caching::InMemoryCacheUser<T> cacheUser(caching::InMemoryCache<T>& cache) {
        return {cache, caches_.at(cache.name())};
    }

    AutoTiming cropTimer() { return timings_.at("crop"); }
    AutoTiming frameTimer() { return timings_.at("frame"); }
    AutoTiming globaliseTimer() { return timings_.at("globalise"); }
    AutoTiming bitmapTimer() { return timings_.at("bitmap"); }
    AutoTiming coefficientTimer() { return timings_.at("coefficient"); }
    AutoTiming sh2gridTimer() { return timings_.at("sh2grid"); }
    AutoTiming grid2gridTimer() { return timings_.at("grid2grid"); }
    AutoTiming vod2uvTimer() { return timings_.at("vod2uv"); }
    AutoTiming computeMatrixTimer() { return timings_.at("computeMatrix"); }
    AutoTiming matrixTimer() { return timings_.at("matrix"); }
    AutoTiming loadCoeffTimer() { return timings_.at("loadCoeff"); }
    AutoTiming createCoeffTimer() { return timings_.at("createCoeff"); }
    AutoTiming calcTimer() { return timings_.at("calc"); }
    AutoTiming nablaTimer() { return timings_.at("nabla"); }
    AutoTiming saveTimer() { return timings_.at("save"); }
    AutoTiming gribEncodingTimer() { return timings_.at("gribEncoding"); }
    AutoTiming gribDecodingTimer() { return timings_.at("gribDecoding"); }

    Timing& gribEncodingTiming() { return timings_.at("gribEncoding"); }
    Timing& gribDecodingTiming() { return timings_.at("gribDecoding"); }

    void report(std::ostream&, const char* indent = "") const;
    void csvHeader(std::ostream&) const;
    void csvRow(std::ostream&) const;
    void encode(eckit::Stream&) const;
    void json(eckit::JSON&) const;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

protected:
    // -- Members
    // None

    // -- Methods
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Members

    std::map<std::string, caching::InMemoryCacheStatistics> caches_;
    std::map<std::string, Timing> timings_;
    std::map<std::string, std::string> descriptions_;

    // -- Methods
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    friend eckit::Stream& operator<<(eckit::Stream& s, const MIRStatistics& x) {
        x.encode(s);
        return s;
    }
};


}  // namespace mir::util
