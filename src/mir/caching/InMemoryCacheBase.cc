/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "eckit/config/Resource.h"

#include "mir/caching/InMemoryCacheBase.h"

#include "eckit/thread/Mutex.h"
#include "eckit/thread/AutoLock.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/log/Bytes.h"

namespace mir {

static eckit::Mutex *local_mutex = 0;
static std::set<InMemoryCacheBase *> *m = 0;
static pthread_once_t once = PTHREAD_ONCE_INIT;

static void init() {
    local_mutex = new eckit::Mutex();
    m = new std::set<InMemoryCacheBase *>();
}


InMemoryCacheBase::InMemoryCacheBase() {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);
    ASSERT(m->find(this) == m->end());
    m->insert(this);
}

InMemoryCacheBase::~InMemoryCacheBase() {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);
    ASSERT(m->find(this) != m->end());
    m->erase(this);
}

unsigned long long InMemoryCacheBase::totalFootprint() {

    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    unsigned long long result = 0;

    for (auto j = m->begin(); j != m->end(); ++j) {
        result += (*j)->footprint();
    }

    return result;
}

void InMemoryCacheBase::checkTotalFootprint() {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    static const unsigned long long maximumCapacity = eckit::Resource<unsigned long long>("mirTotalInMemoryCacheCapacity;$MIR_TOTAL_CACHE_MEMORY_FOOTPRINT", 1L * 1024 * 1024 * 1024);

    bool more = true;
    while (more) {

        more = false;

        unsigned long long totalFootprint = 0;

        for (auto j = m->begin(); j != m->end(); ++j) {
            totalFootprint += (*j)->footprint();
        }

        if (totalFootprint > maximumCapacity) {

            eckit::Log::info() << "CACHE-checkTotalFootprint size="
                               << eckit::Bytes(totalFootprint)
                               << ", max is "
                               <<  eckit::Bytes(maximumCapacity)
                               <<  std::endl;

            for (auto j = m->begin(); j != m->end(); ++j) {
                size_t purged = (*j)->purge(1);
                if (purged) {
                    eckit::Log::info() << "CACHE-checkTotalFootprint purged "
                                       << eckit::Bytes(purged)
                                       << " from "
                                       << (*j)->name()
                                       << std::endl;
                    more = true;
                }
            }
        }

    }
}



}  // namespace mir

