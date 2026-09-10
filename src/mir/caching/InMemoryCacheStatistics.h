// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <iosfwd>

#include "eckit/log/Statistics.h"

#include "mir/caching/InMemoryCacheUsage.h"


namespace eckit {
class Stream;
}


namespace mir::caching {


class InMemoryCacheStatistics : public eckit::Statistics {
public:
    InMemoryCacheStatistics();
    InMemoryCacheStatistics(eckit::Stream&);

    InMemoryCacheStatistics& operator+=(const InMemoryCacheStatistics&);
    InMemoryCacheStatistics& operator/=(size_t);


    size_t hits_;
    size_t misses_;
    size_t evictions_;
    size_t insertions_;
    double oldest_;
    double youngest_;
    InMemoryCacheUsage capacity_;
    InMemoryCacheUsage footprint_;

    size_t unique_;
    InMemoryCacheUsage required_;

    void report(const char* title, std::ostream&, const char* indent = "") const;

    void encode(eckit::Stream&) const;

    friend eckit::Stream& operator<<(eckit::Stream& s, const InMemoryCacheStatistics& x) {
        x.encode(s);
        return s;
    }
};


}  // namespace mir::caching
