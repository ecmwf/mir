/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Peter Bispham
/// @author Tiago Quintino
/// @author Pedro Maciel
/// @date May 2015

#ifndef mir_caching_WeightCache_h
#define mir_caching_WeightCache_h


#include "eckit/container/CacheManager.h"
#include "mir/param/MIRParametrisation.h"
#include "eckit/os/Semaphore.h"

namespace mir {
namespace method {
class WeightMatrix;
}
}

namespace mir {
namespace caching {


class WeightCacheLock {
    eckit::PathName path_;
    eckit::Semaphore lock_;
public:
    WeightCacheLock(const std::string&);
    void lock();
    void unlock();
};

struct WeightCacheTraits {

    typedef WeightCacheLock Locker;
    typedef method::WeightMatrix value_type;

    static const char* name();
    static int version();
    static const char* extension();

    static void save(const eckit::CacheManagerBase& manager, const value_type& W, const eckit::PathName& path);
    static void load(const eckit::CacheManagerBase& manager, value_type& W, const eckit::PathName& path);

};


class WeightCache : public eckit::CacheManager<WeightCacheTraits> {

public:  // methods

    explicit WeightCache(const param::MIRParametrisation& parametrisation);

private: // members

    friend WeightCacheTraits;
};


}  // namespace method
}  // namespace mir


#endif
