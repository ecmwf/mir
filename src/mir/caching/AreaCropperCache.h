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

#include "mir/caching/AreaCacheEntry.h"


namespace mir::caching {


struct AreaCropperCacheTraits {

    using value_type = AreaCacheEntry;
    using Locker     = eckit::CacheManagerFileFlock;

    static const char* name();
    static int version();
    static const char* extension();

    static void save(const eckit::CacheManagerBase&, const value_type&, const eckit::PathName&);
    static void load(const eckit::CacheManagerBase&, value_type&, const eckit::PathName&);
};


class AreaCropperCache : public eckit::CacheManager<AreaCropperCacheTraits> {
public:  // methods
    explicit AreaCropperCache();
};


}  // namespace mir::caching
