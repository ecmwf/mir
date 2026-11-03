// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <string>

#include "mir/caching/InMemoryCacheUsage.h"


namespace eckit {
class JSON;
}


namespace mir::caching {


class InMemoryCacheBase {

public:  // methods
    InMemoryCacheBase();

    InMemoryCacheBase(const InMemoryCacheBase&) = delete;
    InMemoryCacheBase(InMemoryCacheBase&&)      = delete;

    virtual ~InMemoryCacheBase();

    void operator=(const InMemoryCacheBase&) = delete;
    void operator=(InMemoryCacheBase&&)      = delete;

    static InMemoryCacheUsage totalFootprint();

    /// The caches footprint and capacity, by name (of the same name, summed)
    static void json(eckit::JSON&);

    /// Release the caches not in use
    static void purgeAll();

    virtual const std::string& name() const = 0;

protected:
    void checkTotalFootprint();

private:
    virtual InMemoryCacheUsage footprint() const                            = 0;
    virtual InMemoryCacheUsage capacity() const                             = 0;
    virtual InMemoryCacheUsage purge(const InMemoryCacheUsage&, bool force) = 0;
};


}  // namespace mir::caching
