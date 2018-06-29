/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/caching/CroppingCache.h"

#include "eckit/log/Bytes.h"
#include "eckit/io/AutoCloser.h"
#include "eckit/serialisation/FileStream.h"
#include "mir/config/LibMir.h"


namespace mir {
namespace caching {


CroppingCache::CroppingCache():
    eckit::CacheManager<CroppingCacheTraits>(
        "Cropper",  // dummy -- would be used in load() / save() static functions
        LibMir::cacheDir(),
        eckit::Resource<bool>("$MIR_THROW_ON_CACHE_MISS;mirThrowOnCacheMiss", false),
        eckit::Resource<size_t>("$MIR_AREA_CACHE_SIZE", 0)) {
}


void CroppingCacheEntry::print(std::ostream& out) const {
    out << "CroppingCacheEntry[size="
        <<  mapping_.size()
        << ",bbox=" << bbox_
        << ",size=" << eckit::Bytes(sizeof(size_t) * mapping_.size()) << "]";
}


CroppingCacheEntry::~CroppingCacheEntry() = default;


size_t CroppingCacheEntry::footprint() const {
    return sizeof(*this) + mapping_.capacity() * sizeof(size_t);
}


void CroppingCacheEntry::save(const eckit::PathName& path) const {

    eckit::TraceTimer<LibMir> timer("Saving cropping to cache");

    eckit::FileStream f(path, "w"); auto c = closer(f);
    f << bbox_.north();
    f << bbox_.west();
    f << bbox_.south();
    f << bbox_.east();

    f << mapping_.size();
    for (size_t i = 0; i < mapping_.size(); ++i) {
        f << mapping_[i];
    }
}


void CroppingCacheEntry::load(const eckit::PathName& path)  {

    eckit::TraceTimer<LibMir> timer("Loading cropping from cache");

    eckit::FileStream f(path, "r"); auto c = closer(f);

    Latitude n;
    Longitude w;
    Latitude s;
    Longitude e;

    f >> n;
    f >> w;
    f >> s;
    f >> e;

    bbox_ = util::BoundingBox(n, w, s, e);

    size_t size;
    f >> size;

    mapping_.clear();
    mapping_.reserve(size);
    for (size_t i = 0; i < size; ++i) {
        size_t j;
        f >> j;
        mapping_.push_back(j);
    }
}


//--------------------------------------------------------------------------


const char *CroppingCacheTraits::name() {
    return "mir/cropping";
}


int CroppingCacheTraits::version() {
    return 7;
}


const char *CroppingCacheTraits::extension() {
    return ".area";
}


void CroppingCacheTraits::save(const eckit::CacheManagerBase&, const value_type& c, const eckit::PathName& path) {
    c.save(path);
}


void CroppingCacheTraits::load(const eckit::CacheManagerBase&, value_type& c, const eckit::PathName& path) {
    c.load(path);
}


}  // namespace caching
}  // namespace mir

