/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date Apr 2015


#include "mir/action/misc/AreaCropper.h"

#include <iostream>
#include <vector>
#include "eckit/exception/Exceptions.h"
#include "eckit/log/Timer.h"
#include "eckit/memory/ScopedPtr.h"
#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"
#include "atlas/grid/Grid.h"
#include "mir/action/context/Context.h"
#include "mir/caching/CroppingCache.h"
#include "mir/caching/InMemoryCache.h"
#include "mir/data/MIRField.h"
#include "mir/config/LibMir.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"
#include "mir/util/MIRStatistics.h"


namespace mir {
namespace action {


struct LL {
    double lat_;
    double lon_;
    LL(double lat, double lon): lat_(lat), lon_(lon) {}
    bool operator<(const LL &other) const {
        // Order must be like natural scanning mode
        if (lat_ == other.lat_) {
            return lon_ < other.lon_;
        }

        return lat_ > other.lat_;
    }
};


static eckit::Mutex local_mutex;

static InMemoryCache<caching::CroppingCacheEntry> cache("mirArea", 256 * 1024 * 1024, "$MIR_AREA_CACHE_MEMORY_FOOTPRINT");


AreaCropper::AreaCropper(const param::MIRParametrisation &parametrisation):
    Action(parametrisation),
    bbox_(),
    caching_(true) {

    std::vector<double> value;
    ASSERT(parametrisation.get("user.area", value));
    ASSERT(value.size() == 4);

    parametrisation_.get("caching", caching_);

    bbox_ = util::BoundingBox(value[0], value[1], value[2], value[3]);
}


AreaCropper::AreaCropper(const param::MIRParametrisation &parametrisation, const util::BoundingBox &bbox):
    Action(parametrisation),
    bbox_(bbox),
    caching_(true) {
}


AreaCropper::~AreaCropper() {
}


bool AreaCropper::sameAs(const Action& other) const {
    const AreaCropper* o = dynamic_cast<const AreaCropper*>(&other);
    return o && (bbox_ == o->bbox_);
}


void AreaCropper::print(std::ostream &out) const {
    out << "AreaCropper[bbox=" << bbox_ << "]";
}


static void createCroppingCacheEntry(caching::CroppingCacheEntry& c,
                                     const repres::Representation *representation,
                                     const util::BoundingBox &bbox) {
    std::map<LL, size_t> m;

    double n = 0;
    double s = 0;
    double e = 0;
    double w = 0;

    size_t p = 0;
    size_t count = 0;
    bool first = true;
    double lat, lon;

    // Iterator is "unrotated", because the cropping area
    // is expressed in before the rotation is applied
    eckit::ScopedPtr<repres::Iterator> iter(representation->unrotatedIterator());
    const atlas::grid::Domain domain = representation->atlasDomain(bbox);
    while (iter->next(lat, lon)) {
        if (domain.contains(lon, lat)) {

            lon = domain.normalise(lon);
            if (first) {
                n = s = lat;
                e = w = lon;
                first = false;
            } else {
                if     (n < lat) { n = lat; }
                else if(s > lat) { s = lat; }
                if     (e < lon) { e = lon; }
                else if(w > lon) { w = lon; }
            }

            // if(m.find(LL(lat, lon)) != m.end()) {
            //     eckit::Log::debug<LibMir>() << "CROP  duplicate " << lat << ", " << lon << std::endl;
            // }
            m.insert(std::make_pair(LL(lat, lon), p));
            count++;

        }
        p++;
    }

    // Don't support empty results
    if (!m.size()) {
        std::ostringstream oss;
        oss << "Cropping " << *representation << " to " << bbox << " returns no points";
        throw eckit::UserError(oss.str());
    }

    // Make sure we did not visit duplicate points
    ASSERT(count == m.size());


    c.bbox_ = util::BoundingBox(n, w, s, e);

    eckit::Log::info() << "request " << bbox << " domain=" << domain << " bbox=" << c.bbox_ << std::endl;

    c.mapping_.clear();
    c.mapping_.reserve(m.size());
    for (std::map<LL, size_t>::const_iterator j = m.begin(); j != m.end(); ++j) {
        c.mapping_.push_back((*j).second);
    }
}


static const caching::CroppingCacheEntry &getMapping(const std::string& key,
        const repres::Representation *representation,
        const util::BoundingBox &bbox,
        bool caching) {


    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    InMemoryCache<caching::CroppingCacheEntry>::iterator a = cache.find(key);
    if (a != cache.end()) {
        return *a;
    }

    caching::CroppingCacheEntry& c = cache[key];
    if (caching) {
        static caching::CroppingCache disk;

        class CroppingCacheCreator: public caching::CroppingCache::CacheContentCreator {

            const repres::Representation *representation_;
            const util::BoundingBox &bbox_;

            virtual void create(const eckit::PathName& path, caching::CroppingCacheEntry& c) {
                createCroppingCacheEntry(c, representation_, bbox_);
            }

        public:
            CroppingCacheCreator(const repres::Representation *representation,
                                 const util::BoundingBox &bbox):
                representation_(representation),
                bbox_(bbox) {}
        };

        CroppingCacheCreator creator(representation, bbox);
        disk.getOrCreate(key, creator, c);

    } else {

        createCroppingCacheEntry(c, representation, bbox);

    }

    cache.footprint(key, c.footprint());
    return c;

}

static const caching::CroppingCacheEntry &getMapping(const repres::Representation *representation,
        const util::BoundingBox &bbox,
        bool caching) {

    eckit::ScopedPtr<atlas::grid::Grid> gin(representation->atlasGrid()); // This should disapear once we move Representation to atlas
    eckit::MD5 md5;
    md5 << *gin << bbox;

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


void AreaCropper::execute(context::Context & ctx) const {

    // Make sure another thread to no evict anything from the cache while we are using it
    InMemoryCacheUser<caching::CroppingCacheEntry> use(cache, ctx.statistics().areaCroppingCache_);


    data::MIRField& field = ctx.field();

    // Keep a pointer on the original representation, as the one in the field will
    // be changed in the loop
    repres::RepresentationHandle representation(field.representation());
    const caching::CroppingCacheEntry &c = getMapping(representation, bbox_, caching_);

    ASSERT(c.mapping_.size());

    // eckit::Log::debug<LibMir>() << "CROP resulting bbox is: " << c.bbox_ <<
    //                    ", size=" << c.mapping_.size() << std::endl;


    eckit::AutoTiming timing(ctx.statistics().timer_, ctx.statistics().cropTiming_);

    for (size_t i = 0; i < field.dimensions(); i++) {
        const std::vector<double> &values = field.values(i);

        std::vector<double> result;

        result.reserve(c.mapping_.size());

        for (std::vector<size_t>::const_iterator j = c.mapping_.begin(); j != c.mapping_.end(); ++j) {
            result.push_back(values[*j]);
        }

        repres::RepresentationHandle cropped(representation->cropped(c.bbox_));
        // eckit::Log::debug<LibMir>() << *cropped << std::endl;

        if (result.size() == 0) {
            std::ostringstream oss;
            oss << "AreaCropper: failed to crop " << *representation << " with bbox " << c.bbox_ << " cropped=" << *cropped ;
            throw eckit::UserError(oss.str());
        }

        ASSERT(result.size() > 0);
        cropped->validate(result);

        field.representation(cropped);
        field.update(result, i);
    }
}


namespace {
static ActionBuilder< AreaCropper > subAreaCropper("crop.area");
}


}  // namespace action
}  // namespace mir

