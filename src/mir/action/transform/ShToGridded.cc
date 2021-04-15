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


#include "mir/action/transform/ShToGridded.h"

#include <mutex>
#include <ostream>
#include <sstream>

#include "eckit/system/MemoryInfo.h"
#include "eckit/system/SystemInfo.h"
#include "eckit/utils/MD5.h"

#include "mir/action/context/Context.h"
#include "mir/action/transform/TransCache.h"
#include "mir/api/MIREstimation.h"
#include "mir/caching/InMemoryCache.h"
#include "mir/caching/LegendreCache.h"
#include "mir/caching/legendre/LegendreLoader.h"
#include "mir/data/MIRField.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/util/Domain.h"
#include "mir/util/Exceptions.h"
#include "mir/util/MIRStatistics.h"
#include "mir/util/Trace.h"


namespace mir {
namespace action {
namespace transform {


constexpr size_t CAPACITY_MEMORY = 8L * 1024 * 1024 * 1024;
constexpr size_t CAPACITY_SHARED = CAPACITY_MEMORY;
static caching::InMemoryCache<TransCache> trans_cache("mirCoefficient", CAPACITY_MEMORY, CAPACITY_SHARED,
                                                      "$MIR_COEFFICIENT_CACHE");


static atlas::trans::Cache getTransCache(atlas::trans::LegendreCacheCreator& creator, const std::string& key,
                                         const param::MIRParametrisation& parametrisation, context::Context& ctx) {
    auto j = trans_cache.find(key);
    if (j != trans_cache.end()) {
        ASSERT(j->transCache_);
        return j->transCache_;
    }


    // Make sure we have enough space in cache to add new coefficients
    // otherwise we may get killed by OOM thread
    trans_cache.reserve(creator.estimate(), caching::legendre::LegendreLoaderFactory::inSharedMemory(parametrisation));


    eckit::PathName path;
    {
        // Block for timers
        auto timing(ctx.statistics().coefficientTimer());

        class LegendreCacheCreator final : public caching::LegendreCache::CacheContentCreator {

            atlas::trans::LegendreCacheCreator& creator_;
            context::Context& ctx_;

            void create(const eckit::PathName& path, caching::LegendreCacheTraits::value_type& /*ignore*/,
                        bool& saved) override {
                trace::ResourceUsage usage("ShToGridded: create Legendre coefficients");
                auto timing(ctx_.statistics().createCoeffTimer());

                // This will create the cache
                Log::info() << "ShToGridded: create Legendre coefficients '" + path + "'" << std::endl;
                creator_.create(path);

                saved = path.exists();
            }

        public:
            LegendreCacheCreator(atlas::trans::LegendreCacheCreator& creator, context::Context& ctx) :
                creator_(creator), ctx_(ctx) {}

            LegendreCacheCreator(const LegendreCacheCreator&) = delete;
            LegendreCacheCreator& operator=(const LegendreCacheCreator&) = delete;
        };

        static caching::LegendreCache cache;
        LegendreCacheCreator create(creator, ctx);

        int dummy = 0;
        path      = cache.getOrCreate(key, create, dummy);
    }


    TransCache& tc                  = trans_cache[key];
    atlas::trans::Cache& transCache = tc.transCache_;

    {
        trace::ResourceUsage usage("ShToGridded: load Legendre coefficients");
        auto timing(ctx.statistics().loadCoeffTimer());

        Log::info() << "ShToGridded: loading Legendre coefficients '" + path + "'" << std::endl;

        auto before = eckit::system::SystemInfo::instance().memoryUsage();

        tc.loader_ = caching::legendre::LegendreLoaderFactory::build(parametrisation, path);
        ASSERT(tc.loader_);
        Log::debug() << "ShToGridded: LegendreLoader " << *tc.loader_ << std::endl;

        transCache = tc.loader_->transCache();

        auto after = eckit::system::SystemInfo::instance().memoryUsage();
        after.delta(Log::info(), before);

        size_t memory                                    = 0;
        size_t shared                                    = 0;
        (tc.loader_->inSharedMemory() ? shared : memory) = tc.loader_->size();
        trans_cache.footprint(key, caching::InMemoryCacheUsage(memory, shared));
    }

    ASSERT(transCache);
    return transCache;
}


static eckit::Hash::digest_t atlasOptionsDigest(const ShToGridded::atlas_config_t& options) {
    eckit::MD5 h;
    options.hash(h);
    return h.digest();
}


void ShToGridded::transform(data::MIRField& field, const repres::Representation& representation,
                            context::Context& ctx) const {
    static std::mutex local_mutex;
    eckit::AutoLock<std::mutex> lock(local_mutex);  // To protect trans_cache

    // Make sure another thread to no evict anything from the cache while we are using it
    // FIXME check if it should be in ::execute()
    auto cacheUse(ctx.statistics().cacheUser(trans_cache));


    atlas::Grid grid = representation.atlasGrid();
    ASSERT(grid);


    atlas::Domain domain = representation.domain();
    if (cropping_) {

        // bounding box needs adjustment because it can come from the user
        const util::BoundingBox& bbox = cropping_.boundingBox();
        repres::RepresentationHandle cropped(representation.croppedRepresentation(bbox));

        domain = cropped->domain();
    }
    ASSERT(domain);


    const int truncation = int(field.representation()->truncation());
    ASSERT(truncation > 0);


    atlas::trans::LegendreCacheCreator creator(grid, truncation, options_);
    const std::string key(creator.uid());
    ASSERT(!key.empty());

    atlas_trans_t trans;
    try {
        trace::Timer time("ShToGridded::caching", Log::debug());

        bool caching = true;
        parametrisation_.get("caching", caching);

        auto j = trans_cache.find(key);
        if (j != trans_cache.end()) {

            ASSERT(j->transCache_);
            trans = atlas_trans_t(j->transCache_, grid, domain, truncation, options_);
        }
        else if (!creator.supported()) {

            Log::warning() << "ShToGridded: LegendreCacheCreator is not supported for:"
                           << "\n  representation: " << representation << "\n  options: " << options_ << std::endl
                           << "ShToGridded: continuing with hindered performance" << std::endl;

            trans = atlas_trans_t(grid, domain, truncation, options_);
        }
        else if (!caching) {

            auto& entry(trans_cache[key] = creator.create());
            ASSERT(entry.transCache_);
            trans = atlas_trans_t(entry.transCache_, grid, domain, truncation, options_);
        }
        else {

            ASSERT(creator.supported());
            atlas::trans::Cache transCache = getTransCache(creator, key, parametrisation_, ctx);
            ASSERT(transCache);
            trans = atlas_trans_t(transCache, grid, domain, truncation, options_);
        }
    }
    catch (std::exception& e) {
        Log::error() << "ShToGridded::caching: " << e.what() << std::endl;
        trans_cache.erase(key);
        throw;
    }
    ASSERT(trans);

    try {

        auto time(ctx.statistics().sh2gridTimer());
        sh2grid(field, trans, parametrisation_);
    }
    catch (std::exception& e) {
        Log::error() << "ShToGridded::transform: " << e.what() << std::endl;
        throw;
    }
}


ShToGridded::ShToGridded(const param::MIRParametrisation& parametrisation) : Action(parametrisation) {

    // use the 'local' spectral transforms
    std::string type = "local";
    parametrisation.get("atlas-trans-type", type);

    if (!atlas::trans::Trans::hasBackend(type)) {
        std::ostringstream msg;
        msg << "ShToGridded: Atlas/Trans spectral transforms type '" << type
            << "' not supported, available types are: ";
        atlas::trans::Trans::listBackends(msg);
        Log::error() << msg.str() << std::endl;
        throw exception::UserError(msg.str());
    }

    options_.set(atlas::option::type(type));


    // TODO: MIR-183 let Trans decide the best Legendre transform method
    bool flt = false;
    parametrisation.userParametrisation().get("atlas-trans-flt", flt);
    options_.set("flt", flt);
}


ShToGridded::~ShToGridded() = default;


void ShToGridded::print(std::ostream& out) const {
    // We don't want to 'see' the internal options, just if they are set differently
    // (so we know when they change)
    out << "type=" << options_.getString("type") << ",cropping=" << cropping_ << ",options=["
        << atlasOptionsDigest(options_) << "]";
}


void ShToGridded::execute(context::Context& ctx) const {

    repres::RepresentationHandle out(outputRepresentation());

    transform(ctx.field(), *out, ctx);

    if (cropping_) {
        trace::ResourceUsage usage("ShToGridded: cropping");
        auto timing(ctx.statistics().cropTimer());

        const util::BoundingBox& bbox = cropping_.boundingBox();
        ctx.field().representation(out->croppedRepresentation(bbox));
    }
    else {

        ctx.field().representation(out);
    }
}

void ShToGridded::estimate(context::Context& ctx, api::MIREstimation& estimation) const {


    repres::RepresentationHandle out(cropping_ ? outputRepresentation()->croppedRepresentation(cropping_.boundingBox())
                                               : outputRepresentation());

    estimation.numberOfGridPoints(out->numberOfPoints());
    ctx.field().representation(out);
}


bool ShToGridded::mergeWithNext(const Action& next) {

    // make use of the area cropping action downstream (no merge)
    bool canMerge = "local" == options_.getString("type", "?");

    if (!cropping_ && next.canCrop() && canMerge) {
        const util::BoundingBox& bbox = next.outputBoundingBox();

        // if directly followed by cropping go straight to the cropped representation
        if (next.isCropAction()) {

            // Magic super-powers!
            cropping_.boundingBox(bbox);

            return true;
        }

        // otherwise, calculate a bounding box containing the next action's cropping
        std::ostringstream oldAction;
        oldAction << *this;

        // Magic super-powers!
        repres::RepresentationHandle out(outputRepresentation());
        cropping_.boundingBox(out->extendBoundingBox(bbox));

        Log::debug() << "ShToGridded::mergeWithNext: "
                     << "\n   " << oldAction.str() << "\n + " << next << "\n = " << *this << "\n + "
                     << "(...)" << std::endl;
    }
    return false;
}


bool ShToGridded::sameAs(const Action& other) const {
    auto o = dynamic_cast<const ShToGridded*>(&other);
    return (o != nullptr) && atlasOptionsDigest(options_) == atlasOptionsDigest(o->options_);
}


}  // namespace transform
}  // namespace action
}  // namespace mir
