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
#include "eckit/log/Seconds.h"
#include "eckit/log/BigNum.h"
#include "mir/caching/InMemoryCacheStatistics.h"

#include <time.h>
#include <sys/time.h>

namespace mir {

inline static double utime() {
    struct timeval t;
    ::gettimeofday(&t, 0);
    return double(t.tv_sec) + double(t.tv_usec) * 0.000001;
}

template<class T>
InMemoryCache<T>::InMemoryCache(const std::string& name, size_t capacity, const char* variable):
    name_(name),
    capacity_(name + "InMemoryCacheCapacity;"  + variable, capacity),
    users_(0),
    statistics_(0) {
}


template<class T>
InMemoryCache<T>::~InMemoryCache() {
    // std::cout << "Deleting InMemoryCache " << name_ << " capacity=" << capacity_ << ", entries: " << cache_.size() << std::endl;
    // for (typename std::map<std::string, Entry*>::iterator j = cache_.begin(); j != cache_.end(); ++j) {
    //     std::cout << "Deleting InMemoryCache " << name_ << " " << *((*j).second->ptr_) << std::endl;
    //     delete (*j).second;
    // }
}

template<class T>
T* InMemoryCache<T>::find(const std::string& key) const {
    eckit::AutoLock<eckit::Mutex> lock(mutex_);

    typename std::map<std::string, Entry*>::const_iterator j = cache_.find(key);
    if (j != cache_.end()) {
        if (statistics_) {
            statistics_->hits_++;
        }
        (*j).second->hits_++;
        (*j).second->last_ = utime();
        return (*j).second->ptr_.get();
    }
    if (statistics_) {
            statistics_->misses_++;
        }
    return 0;
}


template<class T>
T& InMemoryCache<T>::operator[](const std::string& key) {
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
T& InMemoryCache<T>::insert(const std::string& key, T* ptr) {
    ASSERT(ptr);

    eckit::AutoLock<eckit::Mutex> lock(mutex_);

    if (statistics_) {
        statistics_->insertions_++;
    }
    // std::cout << "Insert in InMemoryCache " << *ptr << std::endl;

    typename std::map<std::string, Entry*>::iterator k = cache_.find(key);
    if (k != cache_.end()) {
        delete (*k).second;
        (*k).second = new Entry(ptr);
        return *ptr;
    }

    if (users_ == 0) {
        purge();
    }

    cache_[key] = new Entry(ptr);

    if (statistics_) {
        keys_.insert(key);
        statistics_->insertions_++;
        statistics_->unique_ = keys_.size();
    }


    return *ptr;

}

template<class T>
void InMemoryCache<T>::purge() {
    while (footprint() > capacity_) {

        double now = utime();
        typename std::map<std::string, Entry*>::iterator best = cache_.begin();
        double m = 0;

        for (typename std::map<std::string, Entry*>::iterator j = cache_.begin(); j != cache_.end(); ++j) {
            double s = score((*j).second->hits_, now - (*j).second->last_, now - (*j).second->insert_);
            if (s > m) {
                m = s;
                best = j;
            }
        }

        if (statistics_) {

            if (m < statistics_->youngest_ || statistics_->youngest_ == 0 ) {
                statistics_->youngest_ = m;
            }

            if (m > statistics_->oldest_) {
                statistics_->oldest_ = m;
            }

            statistics_->evictions_++;
        }
        delete (*best).second;
        cache_.erase(best);
    }
}


template<class T>
size_t InMemoryCache<T>::footprint() const {
    size_t result = 0;
    for (auto j = cache_.begin(); j != cache_.end(); ++j) {
        result += (*j).second->footprint_;

    }
    if(statistics_ && result > statistics_->footprint_) {
        statistics_->footprint_ = result;
    }
    return result;
}

template<class T>
T& InMemoryCache<T>::create(const std::string& key) {
    return insert(key, new T());
}

template<class T>
void InMemoryCache<T>::startUsing(InMemoryCacheStatistics& statistics) {
    eckit::AutoLock<eckit::Mutex> lock(mutex_);
    users_++;
    // TODO: This does not work with threads
    // The statistics will not be correct as each thread
    // overrides statistics_ to their own version
    statistics_ = 0;
    statistics_ = &statistics;
    statistics_->capacity_ = capacity_;
}

template<class T>
void InMemoryCache<T>::stopUsing() {
    eckit::AutoLock<eckit::Mutex> lock(mutex_);
    ASSERT(users_);
    users_--;
    if (users_ == 0) {
        purge();
    }
    // TODO: This does not work with threads
    // The statistics will not be correct as each thread
    // overrides statistics_ to their own version
    statistics_ = 0;
}


template<class T>
void InMemoryCache<T>::erase(const std::string& key) {
    eckit::AutoLock<eckit::Mutex> lock(mutex_);

    typename std::map<std::string, Entry*>::iterator j = cache_.find(key);
    if (j != cache_.end()) {
        delete (*j).second;
        cache_.erase(j);
    }
}

}  // namespace mir

