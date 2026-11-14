// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/caching/InMemoryCacheBase.h"

#include <map>
#include <set>
#include <utility>

#include "eckit/config/Resource.h"
#include "eckit/log/JSON.h"

#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/Mutex.h"


namespace mir::caching {


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

    for (const auto& j : *m) {
        result += j->footprint();
    }

    return result;
}

void InMemoryCacheBase::json(eckit::JSON& j) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    struct usage_t {
        InMemoryCacheUsage footprint;
        InMemoryCacheUsage capacity;
    };

    std::map<std::string, usage_t> caches;
    for (const auto& c : *m) {
        auto& cache = caches[c->name()];
        cache.footprint += c->footprint();
        cache.capacity += c->capacity();
    }

    auto usage = [&j](const InMemoryCacheUsage& u) {
        j.startObject();
        j << "process" << u.memory() << "shared" << u.shared();
        j.endObject();
    };

    j.startObject();
    for (const auto& cache : caches) {
        j << cache.first;
        j.startObject();
        j << "footprint";
        usage(cache.second.footprint);
        j << "capacity";
        usage(cache.second.capacity);
        j.endObject();
    }
    j.endObject();
}


void InMemoryCacheBase::purgeAll() {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    for (const auto& c : *m) {
        c->purge(c->footprint(), false);
    }
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
        for (const auto& j : *m) {
            totalFootprint += j->footprint();
        }

        Log::debug() << "CACHE-checkTotalFootprint size " << totalFootprint << ", max is " << maximumCapacity
                     << std::endl;

        if (totalFootprint > maximumCapacity) {
            InMemoryCacheUsage p = (totalFootprint - maximumCapacity) / m->size();

            for (const auto& j : *m) {
                if (InMemoryCacheUsage purged = j->purge(p, false); purged) {
                    Log::debug() << "CACHE-checkTotalFootprint purged " << purged << " from " << j->name() << std::endl;
                    more = true;
                }
            }
        }
    }
}


}  // namespace mir::caching
