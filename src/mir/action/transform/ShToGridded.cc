/*
 * (C) Copyright 1996-2017 ECMWF.
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
#include "eckit/log/Timer.h"
#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"
#include "atlas/atlas.h"
#include "atlas/grid/Grid.h"
#include "atlas/grid/Structured.h"
#include "atlas/grid/lonlat/RegularLonLat.h"
#include "mir/action/context/Context.h"
#include "mir/action/transform/TransCache.h"
#include "mir/action/transform/TransInitor.h"
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


namespace {


static eckit::Mutex amutex;
static mir::InMemoryCache<TransCache> trans_handles("mirCoefficient",
        8L * 1024 * 1024 * 1024,
        "$MIR_COEFFICIENT_CACHE",
        false); // Don't cleanup at exit: the Fortran part will dump core


static void fillTrans(struct Trans_t &trans,
                      size_t truncation,
                      const atlas::grid::Grid &grid) {
#ifdef ATLAS_HAVE_TRANS
    const atlas::grid::Structured* reduced = dynamic_cast<const atlas::grid::Structured*>(&grid);
    if (!reduced) {
        throw eckit::SeriousBug("Spherical harmonics transforms only supports SH to ReducedGG/RegularGG/RegularLL.");
    }

    const atlas::grid::lonlat::RegularLonLat* latlon = dynamic_cast<const atlas::grid::lonlat::RegularLonLat* >(&grid);


    ASSERT(trans_new(&trans) == 0);

    ASSERT(trans_set_trunc(&trans, truncation) == 0);

    if (latlon) {
        ASSERT(trans_set_resol_lonlat(&trans, latlon->nlon(), latlon->nlat()) == 0);
    } else {

        const std::vector<long>& pl = reduced->pl();
        ASSERT(pl.size());

        std::vector<int> pli(pl.size());
        ASSERT(pl.size() == pli.size());

        for (size_t i = 0; i < pl.size(); ++i) {
            pli[i] = pl[i];
        }

        ASSERT(trans_set_resol(&trans, pli.size(), &pli[0]) == 0);
    }
#else
    throw eckit::SeriousBug("Spherical harmonics transforms are not supported. "
                            "Please recompile ATLAS with TRANS support enabled.");
#endif
}


static void createCoefficients(const eckit::PathName& path,
                               size_t truncation,
                               const atlas::grid::Grid &grid,
                               context::Context& ctx) {
#ifdef ATLAS_HAVE_TRANS
    eckit::AutoTiming timing(ctx.statistics().timer_, ctx.statistics().createCoeffTiming_);

    struct Trans_t tmp_trans;
    fillTrans(tmp_trans, truncation, grid);

    ASSERT(trans_set_write(&tmp_trans, path.asString().c_str()) == 0);
    ASSERT(trans_setup(&tmp_trans) == 0); // This will create the cache

    trans_delete(&tmp_trans);
#else
    throw eckit::SeriousBug("Spherical harmonics transforms are not supported. "
                            "Please recompile ATLAS with TRANS support enabled.");
#endif
}


}  // (anonymous namespace)


void ShToGridded::transform(
        data::MIRField& field,
        const atlas::grid::Grid& grid,
        context::Context& ctx,
        const std::string& key,
        size_t truncation ) const {
#ifdef ATLAS_HAVE_TRANS
    if (trans_handles.find(key) == trans_handles.end()) {

        eckit::PathName path;

        {   // Block for timers

            eckit::AutoTiming timing(ctx.statistics().timer_, ctx.statistics().coefficientTiming_);

            class LegendreCacheCreator: public caching::LegendreCache::CacheContentCreator {

                size_t truncation_;
                const atlas::grid::Grid & grid_;
                context::Context & ctx_;

                virtual void create(const eckit::PathName& path, int& ignore) {
                    createCoefficients(path, truncation_, grid_, ctx_);
                }
            public:
                LegendreCacheCreator(size_t truncation,
                                     const atlas::grid::Grid& grid,
                                     context::Context& ctx):
                    truncation_(truncation), grid_(grid), ctx_(ctx) {}
            };

            static caching::LegendreCache cache;
            LegendreCacheCreator creator(truncation, grid, ctx);

            int dummy = 0;
            path = cache.getOrCreate(key, creator, dummy);
        }

        {   // Block for timers

            eckit::AutoTiming timing(ctx.statistics().timer_, ctx.statistics().loadCoeffTiming_);

            eckit::Timer timer("Loading coefficients");

            TransCache &tc = trans_handles[key];

            struct Trans_t &trans = tc.trans_;
            fillTrans(trans, truncation, grid);

            tc.inited_ = true;
            tc.loader_ = caching::legendre::LegendreLoaderFactory::build(parametrisation_, path);
            // std::cout << "LegendreLoader " << *tc.loader_ << std::endl;

            ASSERT(trans_set_cache(&trans, tc.loader_->address(), tc.loader_->size()) == 0);

            ASSERT(trans_setup(&trans) == 0);
        }

        // trans_handles.footprint(key, after - before);


    }


    eckit::AutoTiming timing(ctx.statistics().timer_, ctx.statistics().sh2gridTiming_);
    eckit::Timer timer("SH2GRID");

    TransCache& tc = trans_handles[key];
    struct Trans_t& trans = tc.trans_;


    // transform sp to gp fields
    sh2grid(trans, field);


    // trans_delete(&trans);
#else
    throw eckit::SeriousBug("Spherical harmonics transforms are not supported. "
                            "Please recompile ATLAS with TRANS support enabled.");
#endif
}


void ShToGridded::transform(data::MIRField& field, const atlas::grid::Grid& grid, context::Context& ctx) const {
    eckit::AutoLock<eckit::Mutex> lock(amutex); // To protect trans_handles

    TransInitor::instance(); // Will init trans if needed

    size_t truncation = field.representation()->truncation();
    std::ostringstream os;
    os << "T" << truncation << ":" << grid.uniqueId();
    std::string key(os.str());

    try {
        transform(field, grid, ctx, key, truncation);
    } catch (std::exception& e) {
        eckit::Log::error() << "Error while running SH2GRID: " << e.what() << std::endl;
        trans_handles.erase(key);
        throw;
    }
}


ShToGridded::ShToGridded(const param::MIRParametrisation &parametrisation):
    Action(parametrisation) {
}


ShToGridded::~ShToGridded() {
}


void ShToGridded::execute(context::Context& ctx) const {

    // Make sure another thread to no evict anything from the cache while we are using it
    InMemoryCacheUser<TransCache> use(trans_handles, ctx.statistics().transHandleCache_);

    repres::RepresentationHandle out(outputRepresentation());
    eckit::ScopedPtr<atlas::grid::Grid> grid(out->atlasGrid());

    transform(ctx.field(), *grid, ctx);

    ctx.field().representation(out);
}


}  // namespace transform
}  // namespace action
}  // namespace mir

