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


#include "mir/caching/InMemoryCacheBase.h"

#include "eckit/config/Resource.h"

#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/Mutex.h"


namespace mir {
namespace caching {


static util::recursive_mutex* local_mutex = nullptr;
static std::set<InMemoryCacheBase*>* m    = nullptr;
static util::once_flag once;


static void init() {
    local_mutex = new util::recursive_mutex();
    m           = new std::set<InMemoryCacheBase*>();
}


InMemoryCacheBase::InMemoryCacheBase() {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    ASSERT(m->find(this) == m->end());
    m->insert(this);
}

InMemoryCacheBase::~InMemoryCacheBase() {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    ASSERT(m->find(this) != m->end());
    m->erase(this);
}

InMemoryCacheUsage InMemoryCacheBase::totalFootprint() {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    InMemoryCacheUsage result;

    for (auto& j : *m) {
        result += j->footprint();
    }

    return result;
}

void InMemoryCacheBase::checkTotalFootprint() {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    constexpr size_t CAPACITY_MEMORY = 1024LL * 1024 * 1024 * 1024 * 1024 * 1024;
    constexpr size_t CAPACITY_SHARED = CAPACITY_MEMORY;
    static eckit::Resource<InMemoryCacheUsage> totalInMemoryCacheCapacity(
        "mirTotalInMemoryCacheCapacity;$MIR_TOTAL_CACHE_MEMORY_FOOTPRINT",
        InMemoryCacheUsage(CAPACITY_MEMORY, CAPACITY_SHARED));

    InMemoryCacheUsage maximumCapacity = totalInMemoryCacheCapacity;

    if (!maximumCapacity && m->empty()) {
        return;
    }

    for (bool more = true; more;) {
        more = false;

        InMemoryCacheUsage totalFootprint;
        for (auto& j : *m) {
            totalFootprint += j->footprint();
        }

        Log::debug() << "CACHE-checkTotalFootprint size " << totalFootprint << ", max is " << maximumCapacity
                     << std::endl;

        if (totalFootprint > maximumCapacity) {
            InMemoryCacheUsage p = (totalFootprint - maximumCapacity) / m->size();

            for (auto& j : *m) {
                InMemoryCacheUsage purged = j->purge(p);
                if (purged) {
                    Log::debug() << "CACHE-checkTotalFootprint purged " << purged << " from " << j->name() << std::endl;
                    more = true;
                }
            }
        }
    }
}


}  // namespace caching
}  // namespace mir
