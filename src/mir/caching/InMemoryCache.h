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
/// @date May 2015

#ifndef mir_caching_InMemoryCache_H
#define mir_caching_InMemoryCache_H

#include <string>
#include <map>

#include "eckit/memory/NonCopyable.h"
#include "eckit/memory/ScopedPtr.h"
#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"

namespace mir {
class InMemoryCacheStatistics;

//----------------------------------------------------------------------------------------------------------------------

template<class T>
class InMemoryCache : public eckit::NonCopyable {

public:  // methods

    typedef T* iterator;

    explicit InMemoryCache(const std::string& name, size_t capacity = 2);

    ~InMemoryCache();

    iterator find(const std::string& key) const;
    T& insert(const std::string& key, T*);
    T& operator[](const std::string& key);

    iterator end() const { return 0; }

    void erase(const std::string& key);

    void startUsing(InMemoryCacheStatistics&);
    void stopUsing();

private:

    void purge();
    T& create(const std::string& key);

    std::string name_;
    size_t capacity_;

    size_t users_;

    mutable InMemoryCacheStatistics* statistics_;
    mutable eckit::Mutex mutex_;
    mutable std::set<std::string> keys_;

    struct Entry {
        eckit::ScopedPtr<T> ptr_;
        size_t access_;
        double last_;
        double insert_;
        Entry(T* ptr): ptr_(ptr), access_(1), last_(::time(0)), insert_(::time(0)) {}
    };

    std::map<std::string, Entry*> cache_;

};

template<class T>
class InMemoryCacheUser {
    InMemoryCache<T>& cache_;
public:
    InMemoryCacheUser(InMemoryCache<T>& cache, InMemoryCacheStatistics& statistics):
        cache_(cache) {
        cache_.startUsing(statistics);
    }
    ~InMemoryCacheUser() {
        cache_.stopUsing();
    }
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace mir

#include "mir/caching/InMemoryCache.cc"

#endif
