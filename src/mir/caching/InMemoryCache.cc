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


#include <sys/time.h>
#include <time.h>

#include "mir/caching/InMemoryCache.h"
#include "mir/caching/InMemoryCacheStatistics.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"


namespace mir {
namespace caching {


inline static double utime() {
    struct timeval t;
    ::gettimeofday(&t, nullptr);
    return double(t.tv_sec) + double(t.tv_usec) * 0.000001;
}


template <class T>
InMemoryCache<T>::InMemoryCache(const std::string& name, size_t memory_capacity, size_t shared_capacity,
                                const char* variable) :
    name_(name),
    capacity_(name + "InMemoryCacheCapacity;" + variable, InMemoryCacheUsage(memory_capacity, shared_capacity)),
    users_(0) {}


template <class T>
InMemoryCache<T>::~InMemoryCache() {
    // Log::debug() << "Deleting InMemoryCache " << name_ << " capacity=" << capacity_ << ", entries: " <<
    // cache_.size() << std::endl;
    for (auto& j : cache_) {
        // Log::debug() << "Deleting InMemoryCache " << name_ << " " << *(j.second->ptr_) << std::endl;
        delete j.second;
    }

    // std::string title = "InMemoryCache(" + name_ + ")";
    // statistics_.report(title.c_str(), Log::debug());
}


template <class T>
T* InMemoryCache<T>::find(const std::string& key) const {
    util::lock_guard<util::recursive_mutex> lock(mutex_);

    auto j = cache_.find(key);
    if (j != cache_.end()) {
        statistics_.hits_++;

        j->second->hits_++;
        j->second->last_ = utime();
        return j->second->ptr_.get();
    }
    // if (statistics_) {
    //     statistics_.misses_++;
    // }
    return nullptr;
}


template <class T>
void InMemoryCache<T>::footprint(const std::string& key, const InMemoryCacheUsage& usage) {
    util::lock_guard<util::recursive_mutex> lock(mutex_);

    Log::debug() << "CACHE-FOOTPRINT-" << name_ << " " << key << " => " << usage << std::endl;


    auto k = cache_.find(key);
    ASSERT(k != cache_.end());
    k->second->footprint_ = usage;
    keys_[key]            = usage;

    footprint();  //  Update stats


    InMemoryCacheUsage result;
    for (auto j = keys_.begin(); j != keys_.end(); ++j) {
        result += j->second;
    }

    statistics_.required_ = result;

    Log::debug() << "CACHE-FOOTPRINT-" << name_ << " total " << footprint() << " required " << result << " capacity "
                 << capacity_ << std::endl;
}


template <class T>
void InMemoryCache<T>::reserve(size_t size, bool inSharedMemory) {
    InMemoryCacheUsage usage(size, inSharedMemory);
    reserve(usage);
}

template <class T>
void InMemoryCache<T>::reserve(const InMemoryCacheUsage& usage) {
    util::lock_guard<util::recursive_mutex> lock(mutex_);

    auto f = footprint();
    auto c = capacity();
    auto u = usage;
    auto p = (f + u) - c;

    Log::debug() << "CACHE-RESERVE-" << name_ << " "
                 << " => " << u << " footprint: " << f << " capacity: " << c << " f+u: " << f + u << " f+u-c: " << p
                 << std::endl;


    if (p) {
        purge(p, true);
    }
}


template <class T>
T& InMemoryCache<T>::operator[](const std::string& key) {
    util::lock_guard<util::recursive_mutex> lock(mutex_);

    T* ptr = find(key);
    return ptr == nullptr ? create(key) : *ptr;
}

static inline double score(size_t /*count*/, double recent, double /*age*/) {
    // count: number of accesses
    // age: age in seconds since creation
    // recent: age in seconds since last access

    // The higher the score, the most likely to be deleted

    // return (double(recent) + double(age)) / double(count);

    return recent;  // LRU
}


template <class T>
T& InMemoryCache<T>::insert(const std::string& key, T* ptr) {
    util::lock_guard<util::recursive_mutex> lock(mutex_);

    ASSERT(ptr);

    statistics_.insertions_++;

    // Log::debug() << "Insert in InMemoryCache " << *ptr << std::endl;

    auto k = cache_.find(key);
    if (k != cache_.end()) {
        NOTIMP;  // Needs to think more about it
        delete k->second;
        k->second  = new Entry(ptr);
        keys_[key] = InMemoryCacheUsage(size_t(1), size_t(0));
        return *ptr;
    }

    if (users_ == 0) {
        purge();
    }

    cache_[key] = new Entry(ptr);
    keys_[key]  = InMemoryCacheUsage(size_t(1), size_t(0));

    statistics_.unique_ = keys_.size();

    return *ptr;
}

template <class T>
void InMemoryCache<T>::purge() {
    auto f = footprint();
    if (f > capacity_) {
        purge(f - capacity_);
    }
}


template <class T>
InMemoryCacheUsage InMemoryCache<T>::footprint() const {
    InMemoryCacheUsage result;

    for (auto j = cache_.begin(); j != cache_.end(); ++j) {
        result += j->second->footprint_;
    }

    if (result > statistics_.footprint_) {
        statistics_.footprint_ = result;
    }

    return result;
}

template <class T>
T& InMemoryCache<T>::create(const std::string& key) {
    return insert(key, new T());
}

template <class T>
void InMemoryCache<T>::startUsing() {
    util::lock_guard<util::recursive_mutex> lock(mutex_);
    users_++;
}

template <class T>
void InMemoryCache<T>::stopUsing(InMemoryCacheStatistics& statistics) {
    util::lock_guard<util::recursive_mutex> lock(mutex_);

    ASSERT(users_);
    users_--;
    if (users_ == 0) {
        purge();
    }

    checkTotalFootprint();

    statistics_.capacity_ = capacity_;
    statistics            = statistics_;
}


template <class T>
void InMemoryCache<T>::erase(const std::string& key) {
    util::lock_guard<util::recursive_mutex> lock(mutex_);

    auto j = cache_.find(key);
    if (j != cache_.end()) {
        delete j->second;
        cache_.erase(j);
    }
}

template <class T>
InMemoryCacheUsage InMemoryCache<T>::capacity() const {
    return capacity_;
}


template <class T>
const std::string& InMemoryCache<T>::name() const {
    return name_;
}


template <class T>
InMemoryCacheUsage InMemoryCache<T>::purge(const InMemoryCacheUsage& amount, bool force) {
    InMemoryCacheUsage purged;

    if (users_ && !force) {
        return purged;
    }

    Log::debug() << "CACHE " << name_ << " purging " << amount << std::endl;

    while (purged < amount) {

        if (cache_.empty()) {
            break;
        }

        double now = utime();
        auto best  = cache_.begin();
        double m   = 0;

        for (auto j = cache_.begin(); j != cache_.end(); ++j) {
            double s = score(j->second->hits_, now - j->second->last_, now - j->second->insert_);
            if (s > m) {
                m    = s;
                best = j;
            }
        }

        if (m < statistics_.youngest_ || statistics_.youngest_ == 0) {
            statistics_.youngest_ = m;
        }

        if (m > statistics_.oldest_) {
            statistics_.oldest_ = m;
        }

        statistics_.evictions_++;


        purged += best->second->footprint_;

        Log::debug() << "CACHE " << name_ << " decache " << best->first << std::endl;
        delete best->second;
        cache_.erase(best);

        Log::debug() << "CACHE " << name_ << " purging " << amount << " purged " << purged << std::endl;
    }

    return purged;
}


}  // namespace caching
}  // namespace mir
