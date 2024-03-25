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
