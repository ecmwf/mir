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

#include "mir/util/BoundingBox.h"
#include "mir/util/IndexMapping.h"


namespace mir::caching {


struct CroppingCacheEntry {

    ~CroppingCacheEntry();

    util::IndexMapping mapping_;
    util::BoundingBox bbox_;

    void print(std::ostream&) const;

    friend std::ostream& operator<<(std::ostream& out, const CroppingCacheEntry& e) {
        e.print(out);
        return out;
    }

    size_t footprint() const;
    const util::BoundingBox& boundingBox() const { return bbox_; }

    void save(const eckit::PathName&) const;
    void load(const eckit::PathName&);
};


struct CroppingCacheTraits {

    using value_type = CroppingCacheEntry;
    using Locker     = eckit::CacheManagerFileFlock;

    static const char* name();
    static int version();
    static const char* extension();

    static void save(const eckit::CacheManagerBase&, const value_type&, const eckit::PathName&);
    static void load(const eckit::CacheManagerBase&, value_type&, const eckit::PathName&);
};


class CroppingCache : public eckit::CacheManager<CroppingCacheTraits> {
public:  // methods
    explicit CroppingCache();
};


}  // namespace mir::caching
