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
#include <vector>

#include "eckit/exception/Exceptions.h"
#include "eckit/log/ResourceUsage.h"
#include "eckit/log/Timer.h"
#include "eckit/system/SystemInfo.h"
#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"
#include "mir/action/context/Context.h"
#include "mir/action/plan/Action.h"
#include "mir/action/transform/TransCache.h"
#include "mir/caching/InMemoryCache.h"
#include "mir/caching/LegendreCache.h"
#include "mir/caching/legendre/LegendreLoader.h"
#include "mir/config/LibMir.h"
#include "mir/data/MIRField.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/util/MIRStatistics.h"


namespace mir {
namespace action {
namespace transform {


static eckit::Mutex amutex;
static mir::InMemoryCache<TransCache> trans_cache("mirCoefficient",
        8L * 1024 * 1024 * 1024,
        8L * 1024 * 1024 * 1024,
        "$MIR_COEFFICIENT_CACHE",
        false); // Don't cleanup at exit: the Fortran part will dump core


static void createCoefficients(const eckit::PathName& path,
                               const ShToGridded::atlas_config_t& options,
                               const repres::Representation& representation,
                               context::Context& ctx) {
#if 0

    eckit::TraceResourceUsage<LibMir> usage("Create legendre coefficients");


    eckit::Log::info() << "Create legendre coefficients "
                       << representation
                       << " => "
                       << path
                       << std::endl;

    eckit::AutoTiming timing(ctx.statistics().timer_, ctx.statistics().createCoeffTiming_);

    struct Trans_t tmp_trans;

    ASSERT(trans_set_write(&tmp_trans, path.asString().c_str()) == 0);
    ASSERT(trans_setup(&tmp_trans) == 0); // This will create the cache

    trans_delete(&tmp_trans);

#endif
}


static TransCache& getTransCache(const param::MIRParametrisation &parametrisation,
                                 const repres::Representation& representation,
                                 context::Context& ctx,
                                 const std::string& key,
                                 const ShToGridded::atlas_config_t& options,
                                 size_t estimate) {


    InMemoryCache<TransCache>::iterator j = trans_cache.find(key);
    if (j != trans_cache.end()) {
        return *j;
    }


    // Make sure we have enough space in cache to add new coefficients
    // otherwise we may get killed by OOM thread
    trans_cache.reserve(estimate, caching::legendre::LegendreLoaderFactory::inSharedMemory(parametrisation));


    eckit::PathName path;

    {   // Block for timers

        eckit::AutoTiming timing(ctx.statistics().timer_, ctx.statistics().coefficientTiming_);

        class LegendreCacheCreator: public caching::LegendreCache::CacheContentCreator {

            ShToGridded::atlas_config_t options_;
            const repres::Representation& representation_;
            context::Context & ctx_;

            void create(const eckit::PathName& path, int& /*ignore*/, bool& /*saved*/) {
                createCoefficients(path, options_, representation_, ctx_);
            }
        public:
            LegendreCacheCreator(const ShToGridded::atlas_config_t& options,
                                 const repres::Representation& representation,
                                 context::Context& ctx):
                options_(options), representation_(representation), ctx_(ctx) {}
        };

        static caching::LegendreCache cache;
        LegendreCacheCreator creator(options, representation, ctx);

        int dummy = 0;
        path = cache.getOrCreate(key, creator, dummy);
    }


    eckit::AutoTiming timing(ctx.statistics().timer_, ctx.statistics().loadCoeffTiming_);

    eckit::Timer timer("Loading coefficients");

    TransCache &tc = trans_cache[key];

    ShToGridded::atlas_trans_t& trans = tc.trans_;

    size_t memory = 0;
    size_t shared = 0;

    {
        eckit::TraceResourceUsage<LibMir> usage("SH2GG LegendreLoaderFactory");

        tc.inited_ = true;
        tc.loader_ = caching::legendre::LegendreLoaderFactory::build(parametrisation, path);
    }
    // eckit::Log::info() << "LegendreLoader " << *tc.loader_ << std::endl;

#if 0
    {
        eckit::TraceResourceUsage<LibMir> usage("SH2GG trans_set_cache");
        ASSERT(trans_set_cache(&trans, tc.loader_->address(), tc.loader_->size()) == 0);
    }

    ASSERT(trans.ndgl > 0 && (trans.ndgl % 2) == 0);

    {
        eckit::TraceResourceUsage<LibMir> usage("SH2GG trans_setup");
        // size_t before = eckit::system::SystemInfo::instance().memoryAllocated();
        ASSERT(trans_setup(&trans) == 0);
        // size_t after = eckit::system::SystemInfo::instance().memoryAllocated();
        // if (after > before) {
        //     eckit::Log::info() << "SH2GG trans_setup memory usage " << (after - before) << " " <<
        //                        double(after - before) / double(path.size()) * 100.0 << "% of coefficients size" << std::endl;
        //     memory += after - before;
        // }
    }
#endif

    memory += tc.loader_->inSharedMemory() ? 0 : size_t(path.size());
    shared += tc.loader_->inSharedMemory() ? size_t(path.size()) : 0;
    trans_cache.footprint(key, InMemoryCacheUsage(memory, shared));


    return tc;


}


void ShToGridded::transform(data::MIRField& field, const repres::Representation& representation, context::Context& ctx) const {
    eckit::AutoLock<eckit::Mutex> lock(amutex); // To protect trans_cache

    // Make sure another thread to no evict anything from the cache while we are using it
    // FIXME check if it should be in ::execute()
    InMemoryCacheUser<TransCache> use(trans_cache, ctx.statistics().transHandleCache_);


    const size_t truncation = field.representation()->truncation();
    ASSERT(truncation);

    const atlas::Grid grid = representation.atlasGrid();
    ASSERT(grid);

    const std::string key =
            "T" + std::to_string(truncation)
            + ":" + "flt" + std::to_string(options_.getBool("flt"))
            + ":" + representation.uniqueName();

    atlas_trans_t trans;
    try {
        eckit::Timer time("ShToGridded::transform: setup", eckit::Log::debug<LibMir>());

        if (local()) {
            trans = atlas_trans_t(grid, int(truncation), options_);
        } else {

            size_t estimate = truncation * truncation * truncation / 2 * sizeof(double);
            const TransCache& tc = getTransCache(parametrisation_,
                                                 representation,
                                                 ctx,
                                                 key,
                                                 options_,
                                                 estimate);
            trans = tc.trans_;
        }

    } catch (std::exception& e) {
        eckit::Log::error() << "ShToGridded::transform: setup: " << e.what() << std::endl;
        trans_cache.erase(key);
        throw;
    }
    ASSERT(trans);

    try {

        eckit::AutoTiming time(ctx.statistics().timer_, ctx.statistics().sh2gridTiming_);
        sh2grid(field, trans, grid);

    } catch (std::exception& e) {
        eckit::Log::error() << "ShToGridded::transform: invtrans: " << e.what() << std::endl;
        throw;
    }
}


ShToGridded::ShToGridded(const param::MIRParametrisation& parametrisation) :
    Action(parametrisation) {

    if (parametrisation.userParametrisation().has("atlas-trans-local")) {
        local(true);
    }

    // TODO: MIR-183 let Trans decide the best Legendre transform method
    bool flt = false;
    parametrisation.userParametrisation().get("atlas-trans-flt", flt);
    options_.set("flt", flt);

    // no partitioning
    options_.set(atlas::option::global());
}


ShToGridded::~ShToGridded() {
}


void ShToGridded::print(std::ostream& out) const {
    out << "ShToGridded=["
            "cropping=" << cropping_
        << ",options=" << options_
        << "]";
}


void ShToGridded::execute(context::Context& ctx) const {

    repres::RepresentationHandle out(outputRepresentation());

    transform(ctx.field(), *out, ctx);

    ctx.field().representation(out);
}


bool ShToGridded::mergeWithNext(const Action& next) {
    if (!cropping_.active()) {
        if (next.isCropAction() || next.isInterpolationAction()) {

            static util::BoundingBox global;
            const util::BoundingBox& bbox = next.croppingBoundingBox();

            if (bbox != global) {
                repres::RepresentationHandle out(outputRepresentation());
                const util::BoundingBox extended = out->extendedBoundingBox(bbox);

                eckit::Log::debug<LibMir>()
                        << "ShToGridded::mergeWithNext: "
                        << "\n\t   " << *this
                        << "\n\t + " << next
                        << "\n\t extendedBoundingBox:"
                        << "\n\t   " << bbox
                        << "\n\t > " << extended
                        << std::endl;

                // Magic super-powers!
                cropping_.boundingBox(extended);
                local(true);

                return true;
            }

        }
    }
    return false;
}


void ShToGridded::local(bool l) {
    if (!l) {
        if (local()) {
            throw eckit::SeriousBug("ShToGridded::local: Atlas/Trans 'local' has been set, cannot revert");
        }
        return;
    }
    options_.set(atlas::option::type("local"));
}


bool ShToGridded::local() const {
    return options_.has("type") && options_.getString("type") == "local";
}


}  // namespace transform
}  // namespace action
}  // namespace mir

