// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "eckit/container/CacheManager.h"


namespace mir::caching {


struct LegendreCacheTraits {

    using value_type = int;  // dummy
    using Locker     = eckit::CacheManagerFileFlock;

    static const char* name();
    static int version();
    static const char* extension();

    static void save(const eckit::CacheManagerBase&, const value_type&, const eckit::PathName&);
    static void load(const eckit::CacheManagerBase&, value_type&, const eckit::PathName&);
};

class LegendreCache : public eckit::CacheManager<LegendreCacheTraits> {
public:  // methods
    LegendreCache();
};


}  // namespace mir::caching
