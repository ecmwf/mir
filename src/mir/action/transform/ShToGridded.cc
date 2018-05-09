/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @author Tiago Quintino
///
/// @date Apr 2015


#include "mir/action/transform/ShToGridded.h"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <vector>

#include "eckit/exception/Exceptions.h"
#include "eckit/geometry/UnitSphere.h"
#include "eckit/log/ResourceUsage.h"
#include "eckit/log/Timer.h"
#include "eckit/system/SystemInfo.h"
#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"
#include "eckit/utils/MD5.h"
#include "mir/action/context/Context.h"
#include "mir/action/transform/TransCache.h"
#include "mir/caching/InMemoryCache.h"
#include "mir/caching/LegendreCache.h"
#include "mir/caching/legendre/LegendreLoader.h"
#include "mir/config/LibMir.h"
#include "mir/data/MIRField.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/util/Angles.h"
#include "mir/util/Domain.h"
#include "mir/util/MIRStatistics.h"


namespace mir {
namespace action {
namespace transform {


static eckit::Mutex amutex;


static InMemoryCache<TransCache> trans_cache("mirCoefficient",
        8L * 1024 * 1024 * 1024,
        8L * 1024 * 1024 * 1024,
        "$MIR_COEFFICIENT_CACHE",
        false); // Don't cleanup at exit: the Fortran part will dump core


static atlas::trans::Cache getTrans(
        atlas::trans::LegendreCacheCreator& creator,
        const std::string& key,
        const param::MIRParametrisation& parametrisation,
        context::Context& ctx ) {


    // Make sure we have enough space in cache to add new coefficients
    // otherwise we may get killed by OOM thread
    trans_cache.reserve(creator.estimate(), caching::legendre::LegendreLoaderFactory::inSharedMemory(parametrisation));


    eckit::PathName path;
    {
        // Block for timers
        eckit::AutoTiming timing(ctx.statistics().timer_, ctx.statistics().coefficientTiming_);

        class LegendreCacheCreator final : public caching::LegendreCache::CacheContentCreator {

            atlas::trans::LegendreCacheCreator& creator_;
            context::Context& ctx_;

            void create(const eckit::PathName& path, caching::LegendreCacheTraits::value_type& /*ignore*/, bool& saved) override {
                eckit::TraceResourceUsage<LibMir> usage("ShToGridded: create coefficients");
                eckit::AutoTiming timing(ctx_.statistics().timer_, ctx_.statistics().createCoeffTiming_);

                // This will create the cache
                creator_.create(path);

                saved = path.exists();
            }
        public:
            LegendreCacheCreator(
                        atlas::trans::LegendreCacheCreator& creator,
                        context::Context& ctx ) :
                creator_(creator),
                ctx_(ctx) {
            }
        };

        static caching::LegendreCache cache;
        LegendreCacheCreator create(creator, ctx);

        int dummy = 0;
        path = cache.getOrCreate(key, create, dummy);
    }


    TransCache& tc = trans_cache[key];
    atlas::trans::Cache& transCache = tc.transCache_;

    {
        eckit::TraceResourceUsage<LibMir> usage("ShToGridded: load coefficients");
        eckit::AutoTiming timing(ctx.statistics().timer_, ctx.statistics().loadCoeffTiming_);

        const eckit::system::MemoryInfo before = eckit::system::SystemInfo::instance().memoryUsage();

        tc.inited_ = true;
        tc.loader_ = caching::legendre::LegendreLoaderFactory::build(parametrisation, path);
        ASSERT(tc.loader_);
        eckit::Log::debug<LibMir>() << "ShToGridded: LegendreLoader " << *tc.loader_ << std::endl;

        transCache = tc.loader_->transCache();

        eckit::system::MemoryInfo after = eckit::system::SystemInfo::instance().memoryUsage();
        after.delta(eckit::Log::info(), before);

        size_t memory = 0;
        size_t shared = 0;
        (tc.loader_->inSharedMemory() ? shared : memory) = tc.loader_->size();
        trans_cache.footprint(key, InMemoryCacheUsage(memory, shared));
    }

    ASSERT(transCache);
    return transCache;
}


static eckit::Hash::digest_t atlasOptionsDigest(const ShToGridded::atlas_config_t& options) {
    eckit::MD5 h;
    options.hash(h);
    return h.digest();
}


void ShToGridded::transform(data::MIRField& field, const repres::Representation& representation, context::Context& ctx) const {
    eckit::AutoLock<eckit::Mutex> lock(amutex); // To protect trans_cache

    // Make sure another thread to no evict anything from the cache while we are using it
    // FIXME check if it should be in ::execute()
    InMemoryCacheUser<TransCache> use(trans_cache, ctx.statistics().transHandleCache_);


    atlas::Grid grid = representation.atlasGrid();
    ASSERT(grid);

    atlas::Domain domain = grid.domain();
    if (cropping_) {
        util::BoundingBox bbox = representation.croppedBoundingBox(cropping_.boundingBox());
        domain = util::Domain(bbox.north(), bbox.west(),
                              bbox.south(), bbox.east());
    }

    const size_t truncation = field.representation()->truncation();
    ASSERT(truncation);

    atlas::trans::LegendreCacheCreator creator(grid, int(truncation), options_);

    const std::string key(creator.uid());
    ASSERT(!key.empty());

    atlas_trans_t trans;
    try {
        eckit::Timer time("ShToGridded::caching", eckit::Log::debug<LibMir>());

        if (!parametrisation_.has("caching")) {

            InMemoryCache<TransCache>::iterator j = trans_cache.find(key);
            if (j != trans_cache.end()) {
                j->transCache_ = creator.create();
            }
            ASSERT(j->transCache_);
            trans = atlas_trans_t(j->transCache_, grid, domain, int(truncation), options_);

        } else {

            atlas::trans::Cache transCache = getTrans(
                        creator,
                        key,
                        parametrisation_,
                        ctx);
            ASSERT(transCache);
            trans = atlas_trans_t(transCache, grid, domain, int(truncation), options_);
        }

    } catch (std::exception& e) {
        eckit::Log::error() << "ShToGridded::caching: " << e.what() << std::endl;
        trans_cache.erase(key);
        throw;
    }
    ASSERT(trans);

    try {

        eckit::AutoTiming time(ctx.statistics().timer_, ctx.statistics().sh2gridTiming_);
        sh2grid(field, trans);

    } catch (std::exception& e) {
        eckit::Log::error() << "ShToGridded::transform: " << e.what() << std::endl;
        throw;
    }
}


ShToGridded::ShToGridded(const param::MIRParametrisation& parametrisation) :
    Action(parametrisation) {

    // use the 'local' spectral transforms
    options_.set(atlas::option::type("local"));


    // TODO: MIR-183 let Trans decide the best Legendre transform method
    bool flt = false;
    parametrisation.userParametrisation().get("atlas-trans-flt", flt);
    options_.set("flt", flt);
}


ShToGridded::~ShToGridded() {
}


void ShToGridded::print(std::ostream& out) const {
    // We don't want to 'see' the internal options, just if they are set differently
    // (so we know when they change)
    out <<  "cropping=" << cropping_
        << ",options=[" << atlasOptionsDigest(options_) << "]";
}


void ShToGridded::execute(context::Context& ctx) const {

    repres::RepresentationHandle out(outputRepresentation());

    transform(ctx.field(), *out, ctx);

    if (cropping_) {
        const util::BoundingBox& bbox = cropping_.boundingBox();
        repres::RepresentationHandle local(out->croppedRepresentation(bbox));

        ctx.field().representation(local);

    } else {

        ctx.field().representation(out);

    }
}


bool ShToGridded::mergeWithNext(const Action& next) {

    // make use of the area cropping action downstream (no merge)
    if (!cropping_ && next.canCrop()) {
        const util::BoundingBox& bbox = next.croppingBoundingBox();

        repres::RepresentationHandle out(outputRepresentation());

        // if directly followed by cropping go straight to the cropped representation
        if (next.isCropAction()) {

            // Magic super-powers!
            cropping_.boundingBox(out->croppedBoundingBox(bbox));

            return true;
        }

        // extend bbox with element diagonals [m], converted to (central) angle
        double radius = 0;
        ASSERT(out->getLongestElementDiagonal(radius));
        ASSERT(radius > 0);

        double c = radius / atlas::util::Earth::radius();
        double angle = util::radian_to_degree(2. * std::asin(0.5 * c));

        // Extend the box, then crop to produce a valid representation
        util::BoundingBox extended = next.extendedBoundingBox(bbox, angle);
        util::BoundingBox best = out->croppedBoundingBox(extended);

        std::ostringstream oldAction;
        oldAction << *this;

        // Magic super-powers!
        cropping_.boundingBox(best);

        eckit::Log::debug<LibMir>()
                << "ShToGridded::mergeWithNext: "
                << "\n   " << oldAction.str()
                << "\n + " << *this
                << "\n = " << next
                << std::endl;

    }
    return false;
}


bool ShToGridded::sameAs(const Action& other) const {
    const ShToGridded* o = dynamic_cast<const ShToGridded*>(&other);
    return o && atlasOptionsDigest(options_) == atlasOptionsDigest(o->options_);
}


}  // namespace transform
}  // namespace action
}  // namespace mir
