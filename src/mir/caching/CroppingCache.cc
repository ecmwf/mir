/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "CroppingCache.h"

#include "eckit/io/BufferedHandle.h"
#include "eckit/log/BigNum.h"
#include "eckit/log/Bytes.h"
#include "eckit/log/Plural.h"
#include "eckit/log/Seconds.h"
#include "eckit/log/Timer.h"
#include "eckit/serialisation/FileStream.h"
#include "mir/api/mir_version.h"
#include "mir/log/MIR.h"


namespace mir {
namespace caching {


CroppingCache::CroppingCache():
    CacheManager("mir/cropping") {
}


const char *CroppingCache::version() const {
    return "1"; // Change me if the cache file structure changes
}


const char *CroppingCache::extension() const {
    return ".area";
}


void CroppingCache::print(std::ostream &s) const {
    s << "CroppingCache[";
    CacheManager::print(s);
    s << ",name=" << name()
      << ",version=" << version()
      << ",extention=" << extension()
      << "]";
}


void CroppingCache::insert(const std::string &key, const CroppingCacheEntry &c) const {

    eckit::PathName tmp = stage(key);

    // eckit::Log::info() << "Inserting cropping in cache : " << tmp << "" << std::endl;

    // eckit::TraceTimer<MIR> timer("Saving cropping to cache");

    eckit::FileStream f(tmp, "w");
    f << c.bbox_.north();
    f << c.bbox_.west();
    f << c.bbox_.south();
    f << c.bbox_.east();

    f << c.mapping_.size();
    for (size_t i = 0; i < c.mapping_.size(); ++i) {
        f << c.mapping_[i];
    }

    ASSERT(commit(key, tmp));
}


bool CroppingCache::retrieve(const std::string &key, CroppingCacheEntry &c) const {

    eckit::PathName path;

    if (!get(key, path))
        return false;

    // eckit::Log::info() << "Found cropping in cache : " << path << "" << std::endl;
    // eckit::TraceTimer<MIR> timer("Loading cropping from cache");

    eckit::FileStream f(path, "r");
    double n, w, s, e;
    f >> n;
    f >> w;
    f >> s;
    f >> e;

    c.bbox_ = util::BoundingBox(n, w, s, e);

    size_t size;
    f >> size;

    c.mapping_.reserve(size);
    for (size_t i = 0; i < size; ++i) {
        size_t j;
        f >> j;
        c.mapping_.push_back(j);
    }

    return true;
}


void CroppingCacheEntry::print(std::ostream& out) const {
    out << "CroppingCacheEntry[size=" <<  mapping_.size() << ",bbox=" << bbox_ << ",size=" << eckit::Bytes(sizeof(size_t) * mapping_.size()) << "]";
}


CroppingCacheEntry::~CroppingCacheEntry() {
    // std::cout << "Delete " << *this << std::endl;
}


}  // namespace method
}  // namespace mir

