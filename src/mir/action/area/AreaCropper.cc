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


#include "mir/action/area/AreaCropper.h"

#include <ostream>
#include <sstream>
#include <vector>

#include "eckit/utils/MD5.h"

#include "mir/action/context/Context.h"
#include "mir/caching/AreaCropperCache.h"
#include "mir/caching/InMemoryCache.h"
#include "mir/config/LibMir.h"
#include "mir/data/MIRField.h"
#include "mir/key/Area.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/MIRStatistics.h"
#include "mir/util/Mutex.h"


namespace mir::action {


constexpr size_t CAPACITY = 256 * 1024 * 1024;
static caching::InMemoryCache<caching::AreaCropperCacheEntry> cache("mirArea", CAPACITY, 0,
                                                                    "$MIR_AREA_CACHE_MEMORY_FOOTPRINT");


struct LL {
    double lat_;
    double lon_;
    LL(Latitude lat, Longitude lon) : lat_(lat.value()), lon_(lon.value()) {}
    bool operator<(const LL& other) const {
        // Order must be like natural scanning mode
        if (lat_ == other.lat_) {
            return lon_ < other.lon_;
        }

        return lat_ > other.lat_;
    }
};


AreaCropper::AreaCropper(const param::MIRParametrisation& parametrisation) :
    Action(parametrisation), caching_(LibMir::caching()) {
    ASSERT(key::Area::get(parametrisation_.userParametrisation(), bbox_));
    parametrisation_.get("caching", caching_);
}


void AreaCropper::crop(const repres::Representation& repres, util::BoundingBox& bbox, util::IndexMapping& mapping) {
    std::map<LL, size_t> m;

    Latitude n  = 0;
    Latitude s  = 0;
    Longitude e = 0;
    Longitude w = 0;

    bool first = true;

    // Iterator is "unrotated", because the cropping area is expressed in before the rotation is applied
    for (const std::unique_ptr<repres::Iterator> it(repres.iterator()); it->next();) {
        const auto& point = it->pointUnrotated();

        // Log::debug() << point << " ====> " << bbox.contains(point) << std::endl;

        if (bbox.contains(point)) {
            const Latitude& lat = point.lat();
            const Longitude lon = point.lon().normalise(bbox.west());

            if (first) {
                n = s = lat;
                e = w = lon;
                first = false;
            }
            else {
                if (n < lat) {
                    n = lat;
                }
                if (s > lat) {
                    s = lat;
                }
                if (e < lon) {
                    e = lon;
                }
                if (w > lon) {
                    w = lon;
                }
            }

            // Make sure we don't visit duplicate points
            ASSERT(m.insert(std::make_pair(LL(lat, lon), it->index())).second);
        }
    }

    // Set mapping (don't support empty results)
    if (m.empty()) {
        std::ostringstream oss;
        oss << "Cropping " << repres << " to " << bbox << " returns no points";
        throw exception::UserError(oss.str());
    }

    mapping.clear();
    mapping.reserve(m.size());
    for (const auto& j : m) {
        mapping.push_back(j.second);
    }

    // Set resulting bounding box
    bbox = util::BoundingBox(n, w, s, e);
}


AreaCropper::~AreaCropper() = default;


bool AreaCropper::sameAs(const Action& other) const {
    const auto* o = dynamic_cast<const AreaCropper*>(&other);
    return (o != nullptr) && (bbox_ == o->bbox_);
}


void AreaCropper::print(std::ostream& out) const {
    out << "AreaCropper[bbox=" << bbox_ << "]";
}


util::BoundingBox AreaCropper::outputBoundingBox() const {
    return bbox_;
}


static void createAreaCropperCacheEntry(caching::AreaCropperCacheEntry& c, const repres::Representation* representation,
                                        const util::BoundingBox& bbox) {

    Log::debug() << "Creating cropping cache entry for " << bbox << std::endl;
    c.bbox_ = bbox;
    c.mapping_.clear();

    // Give a chance to representation-specialised cropping
    if (representation->crop(c.bbox_, c.mapping_)) {
        return;
    }

    AreaCropper::crop(*representation, c.bbox_, c.mapping_);
}


static const caching::AreaCropperCacheEntry& getMapping(const std::string& key,
                                                        const repres::Representation* representation,
                                                        const util::BoundingBox& bbox, bool caching) {
    static util::recursive_mutex local_mutex;
    util::lock_guard<util::recursive_mutex> lock(local_mutex);

    auto a = cache.find(key);
    if (a != cache.end()) {
        return *a;
    }

    auto& c = cache[key];
    if (caching) {
        static caching::AreaCropperCache disk;

        class CacheCreator final : public caching::AreaCropperCache::CacheContentCreator {

            const repres::Representation* representation_;
            const util::BoundingBox& bbox_;

            void create(const eckit::PathName& /*path*/, caching::AreaCropperCacheEntry& c, bool& /*saved*/) final {
                createAreaCropperCacheEntry(c, representation_, bbox_);
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

        createAreaCropperCacheEntry(c, representation, bbox);
    }

    cache.footprint(key, caching::InMemoryCacheUsage(c.footprint(), 0));
    return c;
}


static const caching::AreaCropperCacheEntry& getMapping(const repres::Representation* representation,
                                                        const util::BoundingBox& bbox, bool caching) {
    eckit::MD5 md5;
    md5 << representation->uniqueName() << bbox;

    std::string key(md5);

    try {
        return getMapping(key, representation, bbox, caching);
    }
    catch (...) {

        // Make sure we don't this entry lying around
        cache.erase(key);

        throw;
    }
}


void AreaCropper::execute(context::Context& ctx) const {

    // Make sure another thread to no evict anything from the cache while we are using it
    auto cacheUse(ctx.statistics().cacheUser(cache));
    auto timing(ctx.statistics().cropTimer());

    // Keep a pointer on the original representation, as the one in the field will
    // be changed in the loop
    auto& field = ctx.field();
    repres::RepresentationHandle representation(field.representation());

    const auto& c = getMapping(representation, bbox_, caching_);
    ASSERT_NONEMPTY_AREA_CROP("AreaCropper", !c.mapping_.empty());

    for (size_t i = 0; i < field.dimensions(); i++) {
        const MIRValuesVector& values = field.values(i);

        MIRValuesVector result;
        result.reserve(c.mapping_.size());

        for (const auto& j : c.mapping_) {
            result.push_back(values[j]);
        }

        repres::RepresentationHandle cropped(representation->croppedRepresentation(c.bbox_));
        // Log::debug() << *cropped << std::endl;

        if (result.empty()) {
            std::ostringstream oss;
            oss << "AreaCropper: failed to crop " << *representation << " with bbox " << c.bbox_
                << " cropped=" << *cropped;
            throw exception::UserError(oss.str());
        }

        cropped->validate(result);

        field.representation(cropped);
        field.update(result, i, field.hasMissing());
    }
}


void AreaCropper::estimate(context::Context& ctx, api::MIREstimation& estimation) const {
    repres::RepresentationHandle in(ctx.field().representation());
    repres::RepresentationHandle out(in->croppedRepresentation(bbox_));

    estimateNumberOfGridPoints(ctx, estimation, *out);
    estimateMissingValues(ctx, estimation, *out);

    ctx.field().representation(out);
}


const char* AreaCropper::name() const {
    return "AreaCropper";
}


bool AreaCropper::isCropAction() const {
    return true;
}


bool AreaCropper::canCrop() const {
    return true;
}


static const ActionBuilder<AreaCropper> __action("crop.area");


}  // namespace mir::action
