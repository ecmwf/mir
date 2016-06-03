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


namespace mir {

//----------------------------------------------------------------------------------------------------------------------

template<class T>
class InMemoryCache : public eckit::NonCopyable {

  public:  // methods

    typedef T* iterator;

    explicit InMemoryCache(size_t capacity);
    ~InMemoryCache();

    iterator find(const std::string& key) const;
    T& insert(const std::string& key, T*);
    T& operator[](const std::string& key);

    iterator end() const { return 0; }

private:

    T& create(const std::string& key);

    size_t capacity_;

    struct Entry {
        eckit::ScopedPtr<T> ptr_;
        size_t access_;
        time_t last_;
        time_t insert_;
        Entry(T* ptr): ptr_(ptr), access_(1), last_(::time(0)), insert_(::time(0)) {}
    };

    std::map<std::string, Entry*> cache_;

};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace mir

#include "mir/caching/InMemoryCache.cc"

#endif
