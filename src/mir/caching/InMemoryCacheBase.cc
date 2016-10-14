/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

// #include "eckit/config/Resource.h"
// #include "eckit/log/Seconds.h"
// #include "eckit/log/BigNum.h"
// #include "eckit/log/Bytes.h"

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

    eckit::Log::info() << "CACHE-checkTotalFootprint =>";

    for (auto j = m->begin(); j != m->end(); ++j) {
        result += (*j)->footprint();
        eckit::Log::info() << " " << (*j)->name() << "=" << eckit::Bytes((*j)->footprint()) << std::endl;
    }

    eckit::Log::info() << " = " << eckit::Bytes(result) << std::endl;

    return result;
}

void InMemoryCacheBase::checkTotalFootprint() {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    unsigned long long total = totalFootprint();
    // eckit::Log::info() << "CACHE-checkTotalFootprint => " << eckit::Bytes(total) << std::endl;

}

}  // namespace mir

