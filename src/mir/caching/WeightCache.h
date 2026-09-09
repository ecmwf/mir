// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "eckit/container/CacheManager.h"


namespace mir {
namespace method {
class WeightMatrix;
}
namespace param {
class MIRParametrisation;
}
}  // namespace mir


namespace mir::caching {


struct WeightCacheTraits {

    using value_type = method::WeightMatrix;
    using Locker     = eckit::CacheManagerFileFlock;

    static const char* name();
    static int version();
    static const char* extension();

    static void save(const eckit::CacheManagerBase&, const value_type&, const eckit::PathName&);
    static void load(const eckit::CacheManagerBase&, value_type&, const eckit::PathName&);
};


class WeightCache : public eckit::CacheManager<WeightCacheTraits> {
public:  // methods
    explicit WeightCache(const param::MIRParametrisation&);

    static int version() { return WeightCacheTraits::version(); }

private:  // members
    friend WeightCacheTraits;
};


}  // namespace mir::caching
