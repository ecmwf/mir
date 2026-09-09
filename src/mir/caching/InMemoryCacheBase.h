// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <string>

#include "mir/caching/InMemoryCacheUsage.h"


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

    virtual const std::string& name() const = 0;

protected:
    void checkTotalFootprint();

private:
    virtual InMemoryCacheUsage footprint() const                                    = 0;
    virtual InMemoryCacheUsage capacity() const                                     = 0;
    virtual InMemoryCacheUsage purge(const InMemoryCacheUsage&, bool force = false) = 0;
};


}  // namespace mir::caching
