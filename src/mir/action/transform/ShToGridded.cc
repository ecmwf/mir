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
#include "mir/util/Cropping.h"
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


static ShToGridded::atlas_trans_t getTrans(const param::MIRParametrisation& parametrisation,
                                           const repres::Representation& representation,
                                           atlas::Grid grid,
                                           context::Context& ctx,
                                           const std::string& key,
                                           const ShToGridded::atlas_config_t& options,
                                           size_t truncation) {


    InMemoryCache<TransCache>::iterator j = trans_cache.find(key);
    if (j != trans_cache.end()) {
        return j->trans_;
    }


    // Shortcut for local transforms (use in-memory cache, but not disk)
    if (options.getString("type") == "local") {
        return ShToGridded::atlas_trans_t(grid, int(truncation), options);
    }

    // Make sure we have enough space in cache to add new coefficients
    // otherwise we may get killed by OOM thread

    // TODO: take target grid into consideration
    size_t estimate = truncation * truncation * truncation / 2 * sizeof(double);
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


    return trans;
}


void ShToGridded::transform(data::MIRField& field, const repres::Representation& representation, context::Context& ctx) const {
    eckit::AutoLock<eckit::Mutex> lock(amutex); // To protect trans_cache

    // Make sure another thread to no evict anything from the cache while we are using it
    // FIXME check if it should be in ::execute()
    InMemoryCacheUser<TransCache> use(trans_cache, ctx.statistics().transHandleCache_);


    const size_t truncation = field.representation()->truncation();

    const atlas::Grid grid = unstructured_ ?
                atlas::grid::UnstructuredGrid(representation.atlasGrid()) :
                representation.atlasGrid();

    const std::string key =
            "T" + std::to_string(truncation)
            + ":" + "flt" + std::to_string(options_.getBool("flt"))
            + ":" + representation.uniqueName();

    try {
        atlas_trans_t trans = getTrans(parametrisation_,
                                       representation,
                                       grid,
                                       ctx,
                                       key,
                                       options_,
                                       truncation);
        {
            eckit::AutoTiming time(ctx.statistics().timer_, ctx.statistics().sh2gridTiming_);

            sh2grid(field, trans, grid);
        }

    } catch (std::exception& e) {
        eckit::Log::error() << "ShToGridded::transform: " << e.what() << std::endl;
        trans_cache.erase(key);
        throw;
    }
}


ShToGridded::ShToGridded(const param::MIRParametrisation& parametrisation) :
    Action(parametrisation),
    radius_(0.) {

    bool local_ = false;
    parametrisation.userParametrisation().get("atlas-trans-local", local_);

    unstructured_ = false;
    parametrisation.userParametrisation().get("atlas-trans-unstructured-grid", unstructured_);

    if (unstructured_ || local_) {
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
        << ",radius=" << radius_
        << ",unstructured=" << unstructured_
        << ",options=" << options_
        << "]";
}


void ShToGridded::execute(context::Context& ctx) const {

    repres::RepresentationHandle out(outputRepresentation());

    transform(ctx.field(), *out, ctx);

    ctx.field().representation(out);
}


bool ShToGridded::mergeWithNext(const Action& next) {
    if (next.isCropAction() || next.isInterpolationAction()) {

        static util::BoundingBox global;

        util::BoundingBox bbox = next.croppingBoundingBox();
        if (bbox != global) {

            eckit::Log::debug<LibMir>()
                    << "ShToGridded::mergeWithNext: "
                    << "\n\t" "   " << *this
                    << "\n\t" " + " << next
                    << std::endl;

            // NOTE: not necessary, just a Gaussian grid condition
            repres::RepresentationHandle out(outputRepresentation());
            radius_ = 0.;
            ASSERT(out->getLongestElementDiagonal(radius_));
            ASSERT(eckit::types::is_strictly_greater(radius_, 0.));

            // Magic super-powers!
            cropping_.boundingBox(bbox);
            local();

            return true;
        }
    }
    return false;
}


void ShToGridded::local(bool l) {
    if (!l && local()) {
        throw eckit::SeriousBug("ShToGridded::local: trans 'local' has been set, cannot revert");
    }
    options_. set(atlas::option::type(l ? "local" : "ifs"));
}


bool ShToGridded::local() const {
    return options_.getString("type") == "local";
}


}  // namespace transform
}  // namespace action
}  // namespace mir

