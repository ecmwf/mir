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


#include "mir/action/area/AreaMasker.h"

#include <ostream>
#include <sstream>

#include "eckit/utils/MD5.h"

#include "mir/action/context/Context.h"
#include "mir/caching/AreaMaskerCache.h"
#include "mir/caching/InMemoryCache.h"
#include "mir/data/MIRField.h"
#include "mir/repres/Representation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/MIRStatistics.h"
#include "mir/util/Mutex.h"


namespace mir::action {


constexpr size_t CAPACITY = 256 * 1024 * 1024;
static caching::InMemoryCache<caching::AreaCacheEntry> cache("mirArea", CAPACITY, 0,
                                                             "$MIR_AREA_CACHE_MEMORY_FOOTPRINT");


void AreaMasker::print(std::ostream& out) const {
    out << "AreaMasker[bbox=" << boundingBox() << "]";
}


bool AreaMasker::sameAs(const Action& other) const {
    const auto* o = dynamic_cast<const AreaMasker*>(&other);
    return (o != nullptr) && Area::sameAs(other);
}


util::BoundingBox AreaMasker::outputBoundingBox() const {
    return boundingBox();
}


static void create_cache_entry(caching::AreaCacheEntry& c, const repres::Representation* representation,
                               const util::BoundingBox& bbox) {
    Log::debug() << "Creating area masker cache entry for " << bbox << std::endl;
    c.bbox_ = bbox;
    c.mapping_.clear();

    AreaMasker::mask(*representation, c.bbox_, c.mapping_);
}


static const caching::AreaCacheEntry& get_cache_entry(const std::string& key,
                                                      const repres::Representation* representation,
                                                      const util::BoundingBox& bbox, bool caching) {
    static util::recursive_mutex local_mutex;
    util::lock_guard<util::recursive_mutex> lock(local_mutex);

    auto* a = cache.find(key);
    if (a != cache.end()) {
        return *a;
    }

    auto& c = cache[key];
    if (caching) {
        static caching::AreaMaskerCache disk;

        class CacheCreator final : public caching::AreaMaskerCache::CacheContentCreator {

            const repres::Representation* representation_;
            const util::BoundingBox& bbox_;

            void create(const eckit::PathName& /*path*/, caching::AreaCacheEntry& c, bool& /*saved*/) final {
                create_cache_entry(c, representation_, bbox_);
            }

        public:
            CacheCreator(const repres::Representation* representation, const util::BoundingBox& bbox) :
                representation_(representation), bbox_(bbox) {}
            ~CacheCreator() override = default;

            CacheCreator(const CacheCreator&)            = delete;
            CacheCreator(CacheCreator&&)                 = delete;
            CacheCreator& operator=(const CacheCreator&) = delete;
            CacheCreator& operator=(CacheCreator&&)      = delete;
        };

        CacheCreator creator(representation, bbox);
        disk.getOrCreate(key, creator, c);
    }
    else {

        create_cache_entry(c, representation, bbox);
    }

    cache.footprint(key, caching::InMemoryCacheUsage(c.footprint(), 0));
    return c;
}


static const caching::AreaCacheEntry& get_cache_entry(const repres::Representation* representation,
                                                      const util::BoundingBox& bbox, bool caching) {
    eckit::MD5 md5;
    md5 << representation->uniqueName() << bbox << caching::AreaMaskerCacheTraits::extension();

    std::string key(md5);

    try {
        return get_cache_entry(key, representation, bbox, caching);
    }
    catch (...) {

        // Make sure we don't this entry lying around
        cache.erase(key);

        throw;
    }
}


void AreaMasker::execute(context::Context& ctx) const {
    // Make sure another thread to no evict anything from the cache while we are using it
    auto cacheUse(ctx.statistics().cacheUser(cache));
    auto timing(ctx.statistics().cropTimer());

    auto& field = ctx.field();
    repres::RepresentationHandle representation(field.representation());

    const auto& c = get_cache_entry(representation, boundingBox(), caching());
    ASSERT_NONEMPTY_AREA("AreaMasker", !c.mapping_.empty());

    auto missingValue = field.missingValue();

    for (size_t i = 0; i < field.dimensions(); i++) {
        const MIRValuesVector& values = field.values(i);

        MIRValuesVector result(values.size(), missingValue);

        for (const auto& j : c.mapping_) {
            result[j] = values[j];
        }

        if (result.empty()) {
            std::ostringstream oss;
            oss << "AreaMasker: failed to mask " << *representation << " with bbox " << c.bbox_;
            throw exception::UserError(oss.str());
        }

        field.update(result, i, true);
    }
}


void AreaMasker::estimate(context::Context& ctx, api::MIREstimation& estimation) const {
    NOTIMP;
}


const char* AreaMasker::name() const {
    return "AreaMasker";
}


static const ActionBuilder<AreaMasker> __action("mask.area");


}  // namespace mir::action
