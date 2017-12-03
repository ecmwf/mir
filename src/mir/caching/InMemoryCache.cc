/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include <time.h>
#include <sys/time.h>

#include "eckit/log/BigNum.h"
#include "eckit/log/Bytes.h"
#include "eckit/log/Seconds.h"
#include "eckit/thread/AutoLock.h"

#include "mir/caching/InMemoryCache.h"
#include "mir/caching/InMemoryCacheStatistics.h"

namespace mir {

inline static double utime() {
    struct timeval t;
    ::gettimeofday(&t, 0);
    return double(t.tv_sec) + double(t.tv_usec) * 0.000001;
}

template<class T>
InMemoryCache<T>::InMemoryCache(const std::string& name, unsigned long long capacity, const char* variable, bool cleanupAtExit):
    name_(name),
    capacity_(name + "InMemoryCacheCapacity;"  + variable, capacity),
    cleanupAtExit_(cleanupAtExit),
    users_(0) {
}


template<class T>
InMemoryCache<T>::~InMemoryCache() {
    if (cleanupAtExit_) {
        // std::cerr << "Deleting InMemoryCache " << name_ << " capacity=" << capacity_ << ", entries: " << cache_.size() << std::endl;
        for (auto j = cache_.begin(); j != cache_.end(); ++j) {
            // std::cerr << "Deleting InMemoryCache " << name_ << " " << *((*j).second->ptr_) << std::endl;
            delete (*j).second;
        }
    }

    // std::string title = "InMemoryCache(" + name_ + ")";
    // statistics_.report(title.c_str(), eckit::Log::info());
}


template<class T>
T* InMemoryCache<T>::find(const std::string & key) const {
    eckit::AutoLock<eckit::Mutex> lock(mutex_);

    auto j = cache_.find(key);
    if (j != cache_.end()) {
        statistics_.hits_++;

        (*j).second->hits_++;
        (*j).second->last_ = utime();
        return (*j).second->ptr_.get();
    }
    // if (statistics_) {
    //     statistics_.misses_++;
    // }
    return 0;
}

template<class T>
void InMemoryCache<T>::footprint(const std::string & key, size_t size, bool shared) {
    eckit::AutoLock<eckit::Mutex> lock(mutex_);

    eckit::Log::info() << "CACHE-FOOTPRINT-" << name_ << " " << key << " => " << eckit::Bytes(size) << std::endl;


    auto j = cache_.find(key);
    ASSERT(j != cache_.end());
    (*j).second->footprint_ = size;
    keys_[key] = size;

    footprint(); //  Update stats


    unsigned long long result = 0;
    for (auto j = keys_.begin(); j != keys_.end(); ++j) {
        result += (*j).second;
    }

    statistics_.required_ = result;

    eckit::Log::info() << "CACHE-FOOTPRINT-" << name_
                       << " total " << eckit::Bytes(footprint())
                       << " required " << eckit::Bytes(result)
                       << " capacity " << eckit::Bytes(capacity_)
                       << std::endl;


}



template<class T>
T& InMemoryCache<T>::operator[](const std::string & key) {
    eckit::AutoLock<eckit::Mutex> lock(mutex_);

    T* ptr = find(key);
    if (ptr) {
        return *ptr;
    }
    return create(key);
}

static inline double score(size_t count, double recent, double age) {
    // count: number of accesses
    // age: age in seconds since creation
    // recent: age in seconds since last access

    // The higher the score, the most likely to be deleted

    // return (double(recent) + double(age)) / double(count);

    return recent; // LRU
}


template<class T>
T& InMemoryCache<T>::insert(const std::string & key, T * ptr) {
    ASSERT(ptr);

    eckit::AutoLock<eckit::Mutex> lock(mutex_);

    statistics_.insertions_++;

    // eckit::Log::info() << "Insert in InMemoryCache " << *ptr << std::endl;

    auto k = cache_.find(key);
    if (k != cache_.end()) {
        NOTIMP; // Needs to think more about it
        delete (*k).second;
        (*k).second = new Entry(ptr);
        keys_[key] = 1;
        return *ptr;
    }

    if (users_ == 0) {
        purge();
    }

    cache_[key] = new Entry(ptr);

    keys_[key] = 1;
    statistics_.unique_ = keys_.size();


    return *ptr;

}

template<class T>
void InMemoryCache<T>::purge() {
    while (footprint() > capacity_) {
        if (!purge(1)) {
            break;
        }
    }
}


template<class T>
unsigned long long InMemoryCache<T>::footprint() const {
    unsigned long long result = 0;
    for (auto j = cache_.begin(); j != cache_.end(); ++j) {
        result += (*j).second->footprint_;

    }
    if (result > statistics_.footprint_) {
        statistics_.footprint_ = result;
    }
    return result;
}

template<class T>
T& InMemoryCache<T>::create(const std::string & key) {
    return insert(key, new T());
}

template<class T>
void InMemoryCache<T>::startUsing() {
    eckit::AutoLock<eckit::Mutex> lock(mutex_);
    users_++;
}

template<class T>
void InMemoryCache<T>::stopUsing(InMemoryCacheStatistics & statistics) {
    eckit::AutoLock<eckit::Mutex> lock(mutex_);
    ASSERT(users_);
    users_--;
    if (users_ == 0) {
        purge();
    }
    checkTotalFootprint();
    statistics = statistics_;
}


template<class T>
void InMemoryCache<T>::erase(const std::string & key) {
    eckit::AutoLock<eckit::Mutex> lock(mutex_);

    auto j = cache_.find(key);
    if (j != cache_.end()) {
        delete (*j).second;
        cache_.erase(j);
    }
}

template<class T>
unsigned long long InMemoryCache<T>::capacity() const {
    return capacity_;
}


template<class T>
const std::string& InMemoryCache<T>::name() const {
    return name_;
}


template<class T>
size_t InMemoryCache<T>::purge(size_t count) {
    eckit::AutoLock<eckit::Mutex> lock(mutex_);

    if (users_) {
        return 0;
    }

    size_t purged = 0;

    for (size_t i = 0; i < count; i++) {

        if (cache_.empty()) {
            break;
        }

        double now = utime();
        auto best = cache_.begin();
        double m = 0;

        for (auto j = cache_.begin(); j != cache_.end(); ++j) {
            double s = score((*j).second->hits_, now - (*j).second->last_, now - (*j).second->insert_);
            if (s > m) {
                m = s;
                best = j;
            }
        }



        if (m < statistics_.youngest_ || statistics_.youngest_ == 0 ) {
            statistics_.youngest_ = m;
        }

        if (m > statistics_.oldest_) {
            statistics_.oldest_ = m;
        }

        statistics_.evictions_++;


        purged += (*best).second->footprint_;

        eckit::Log::info() << name_ << " decache " << (*best).first << std::endl;
        delete (*best).second;
        cache_.erase(best);

    }

    return purged;

}


}  // namespace mir

