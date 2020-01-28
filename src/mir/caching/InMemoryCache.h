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


#ifndef mir_caching_InMemoryCache_h
#define mir_caching_InMemoryCache_h

#include <memory>

#include "eckit/config/Resource.h"
#include "eckit/thread/Mutex.h"

#include "mir/caching/InMemoryCacheBase.h"
#include "mir/caching/InMemoryCacheStatistics.h"


namespace mir {
namespace caching {


template<class T>
class InMemoryCache : public InMemoryCacheBase {

public:  // methods

    using iterator = T*;

    explicit InMemoryCache(const std::string& name,
                           size_t memory_capacity,
                           size_t shared_capacity,
                           const char* variable,
                           bool cleanupAtExit = true);

    ~InMemoryCache();

    iterator find(const std::string& key) const;
    T& insert(const std::string& key, T*);
    T& operator[](const std::string& key);

    iterator end() const { return 0; }

    void footprint(const std::string& key, const InMemoryCacheUsage&);

    void reserve(size_t size, bool inSharedMemory);
    void reserve(const InMemoryCacheUsage&);

    void erase(const std::string& key);

    void startUsing();
    void stopUsing(InMemoryCacheStatistics&);

private:

    void purge();
    T& create(const std::string& key);

    virtual InMemoryCacheUsage footprint() const;
    virtual InMemoryCacheUsage capacity() const;
    virtual InMemoryCacheUsage purge(const InMemoryCacheUsage&, bool force=false);
    virtual const std::string& name() const;

    std::string name_;
    eckit::Resource<InMemoryCacheUsage> capacity_;
    bool cleanupAtExit_;

    size_t users_;

    mutable InMemoryCacheStatistics statistics_;
    mutable eckit::Mutex mutex_;
    mutable std::map<std::string, InMemoryCacheUsage> keys_;

    struct Entry {

        std::unique_ptr<T> ptr_;
        size_t hits_;
        double last_;
        double insert_;
        InMemoryCacheUsage footprint_;

        Entry(T* ptr): ptr_(ptr),
            hits_(1),
            last_(::time(0)),
            insert_(::time(0)),
            footprint_(size_t(1), size_t(0)) {}
    };

    std::map<std::string, Entry*> cache_;

};

template<class T>
class InMemoryCacheUser {
    InMemoryCache<T>& cache_;
    InMemoryCacheStatistics& statistics_;
public:
    InMemoryCacheUser(InMemoryCache<T>& cache, InMemoryCacheStatistics& statistics):
        cache_(cache),
        statistics_(statistics) {
        cache_.startUsing();
    }

    ~InMemoryCacheUser() {
        cache_.stopUsing(statistics_);
    }
};


}  // namespace caching
}  // namespace mir


#include "mir/caching/InMemoryCache.cc"


#endif
