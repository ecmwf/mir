/*
 * (C) Copyright 1996-2016 ECMWF.
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


#include "mir/action/transform/Sh2Gridded.h"

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

#ifdef ATLAS_HAVE_TRANS
#include "transi/trans.h"
#endif


namespace mir {
namespace action {
namespace transform {

namespace {
#ifdef ATLAS_HAVE_TRANS


static eckit::Mutex amutex;
static mir::InMemoryCache<TransCache> trans_handles("mirCoefficient",
        8L * 1024 * 1024 * 1024,
        "$MIR_COEFFICIENT_CACHE",
        false); // Don't cleanup at exit: the Fortran part will dump core

static void fillTrans(struct Trans_t &trans,
                      size_t truncation,
                      const atlas::grid::Grid &grid) {

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
}

static void createCoefficients(const eckit::PathName& path,
                               size_t truncation,
                               const atlas::grid::Grid &grid,
                               context::Context& ctx) {
    eckit::AutoTiming timing(ctx.statistics().timer_, ctx.statistics().createCoeffTiming_);

    struct Trans_t tmp_trans;
    fillTrans(tmp_trans, truncation, grid);

    ASSERT(trans_set_write(&tmp_trans, path.asString().c_str()) == 0);
    ASSERT(trans_setup(&tmp_trans) == 0); // This will create the cache

    trans_delete(&tmp_trans);
}

static void transform(
        const std::string& key,
        const param::MIRParametrisation& parametrisation,
        size_t truncation,
        data::MIRField& field,
        const atlas::grid::Grid& grid,
        context::Context& ctx) {

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
                                     const atlas::grid::Grid & grid,
                                     context::Context & ctx):
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
            tc.loader_ = caching::legendre::LegendreLoaderFactory::build(parametrisation, path);
            // std::cout << "LegendreLoader " << *tc.loader_ << std::endl;

            ASSERT(trans_set_cache(&trans, tc.loader_->address(), tc.loader_->size()) == 0);

            ASSERT(trans_setup(&trans) == 0);
        }

        // trans_handles.footprint(key, after - before);


    }

    eckit::AutoTiming timing(ctx.statistics().timer_, ctx.statistics().sh2gridTiming_);
    eckit::Timer timer("SH2GRID");


    // Transform sp to gp fields =====================================

    TransCache& tc = trans_handles[key];
    struct Trans_t& trans = tc.trans_;

    size_t number_of_fields = field.dimensions();
    ASSERT(number_of_fields > 0);
    ASSERT(trans.myproc == 1);
    ASSERT(trans.nspec2g == int(field.values(0).size()));


    // set input & output working area (avoid copies if transforming one field only)
    bool vod2uv = false;
    parametrisation.get("vod2uv", vod2uv);
    ASSERT(!vod2uv || number_of_fields == 2);

    std::vector<double> output(number_of_fields * size_t(trans.ngptotg));
    std::vector<double> input;
    if (!vod2uv && number_of_fields > 1) {
        long size = long(field.values(0).size());
        input.resize(number_of_fields * size_t(size));

        // spectral coefficients are "interlaced"
        for (size_t i = 0; i < number_of_fields; i++) {
            const std::vector<double>& values = field.values(i);
            ASSERT(int(values.size()) == trans.nspec2g);

            for (size_t j = 0; j < size_t(size); ++j) {
                input[ j*number_of_fields + i ] = values[j];
            }
        }
    }


    // transform
    struct InvTrans_t invtrans = new_invtrans(&trans);
    invtrans.rgp = output.data();
    if (vod2uv) {
        invtrans.nvordiv = 1;
        invtrans.rspvor  = field.values(0).data();
        invtrans.rspdiv  = field.values(1).data();
    } else {
        invtrans.nscalar   = int(number_of_fields);
        invtrans.rspscalar = number_of_fields > 1? input.data() : field.values(0).data();
    }
    ASSERT(trans_invtrans(&invtrans) == 0);


    // set field values (again, avoid copies for one field only)
    if (number_of_fields == 1) {
        field.update(output, 0);
    } else {
        std::vector<double>::const_iterator here = output.begin();
        for (size_t i = 0; i < number_of_fields; i++) {
            std::vector<double> output_field(here, here + trans.ngptotg);

            field.update(output_field, i);
            here += trans.ngptotg;
        }

        if (vod2uv) {
            long id_u = 131;
            long id_v = 132;
            parametrisation.get("transform.vod2uv.u", id_u);
            parametrisation.get("transform.vod2uv.v", id_v);

            field.metadata(0, "paramId", id_u);
            field.metadata(1, "paramId", id_v);
        }
    }


    // trans_delete(&trans);
}
#endif


static void transform(
        const param::MIRParametrisation& parametrisation,
        data::MIRField& field,
        const atlas::grid::Grid& grid,
        context::Context& ctx ) {
    eckit::AutoLock<eckit::Mutex> lock(amutex); // To protect trans_handles

    TransInitor::instance(); // Will init trans if needed

    size_t truncation = field.representation()->truncation();
    std::ostringstream os;
    os << "T" << truncation << ":" << grid.uniqueId();
    std::string key(os.str());

    try {
        transform(key, parametrisation, truncation, field, grid, ctx);
    } catch (std::exception& e) {
        eckit::Log::error() << "Error while running SH2GRID: " << e.what() << std::endl;
        trans_handles.erase(key);
        throw;
    }
}


}  // (anonymous namespace)


Sh2Gridded::Sh2Gridded(const param::MIRParametrisation &parametrisation):
    Action(parametrisation) {
}


Sh2Gridded::~Sh2Gridded() {
}


void Sh2Gridded::execute(context::Context& ctx) const {

    // Make sure another thread to no evict anything from the cache while we are using it
    InMemoryCacheUser<TransCache> use(trans_handles, ctx.statistics().transHandleCache_);

    repres::RepresentationHandle out(outputRepresentation());
    eckit::ScopedPtr<atlas::grid::Grid> grid(out->atlasGrid());

    transform(parametrisation_, ctx.field(), *grid, ctx);

    ctx.field().representation(out);
}


}  // namespace transform
}  // namespace action
}  // namespace mir

