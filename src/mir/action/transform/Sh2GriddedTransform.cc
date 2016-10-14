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
/// @date Apr 2015

// #include <malloc.h>

#include "mir/action/transform/Sh2GriddedTransform.h"

#include <iostream>
#include <vector>

#include "atlas/atlas.h"
#include "atlas/grid/Grid.h"
#include "atlas/grid/Structured.h"
#include "atlas/grid/lonlat/RegularLonLat.h"
#include "atlas/grid/grids.h"

#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/log/Timer.h"
#include "eckit/utils/MD5.h"
#include "eckit/io/FileLock.h"
#include "eckit/os/Malloc.h"

#include "mir/action/context/Context.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/caching/LegendreCache.h"
#include "mir/caching/LegendreLoader.h"
#include "mir/config/LibMir.h"
#include "mir/util/MIRStatistics.h"
#include "mir/caching/InMemoryCache.h"
#include "mir/data/MIRField.h"

#ifdef ATLAS_HAVE_TRANS
#include "transi/trans.h"


class TransInitor {
public:
    TransInitor() {
        trans_use_mpi(false); // So that even if MPI is enabled, we don't use it.
        trans_init();
    }
    ~TransInitor() {
        trans_finalize();
    }
};

struct TransCache {

    bool inited_;
    struct Trans_t trans_;
    mir::caching::LegendreLoader *loader_;

    TransCache():
        inited_(false),
        loader_(0) {}

    void print(std::ostream& s) const {
        s << "TransCache[";
        if (loader_) s << *loader_;
        s << "]";
    }

    friend std::ostream& operator<<(std::ostream& out, const TransCache& e) {
        e.print(out);
        return out;
    }

    ~TransCache() {
        if (inited_) {
            std::cout << "Delete " << *this << std::endl;
            trans_delete(&trans_);
        }
        else {
            std::cout << "Not Deleting " << *this << std::endl;

        }
        delete loader_;
    }
};


static eckit::Mutex amutex;
static mir::InMemoryCache<TransCache> trans_handles("mirCoefficient", 8L * 1024 * 1024 * 1024, "$MIR_COEFFICIENT_CACHE");

#endif


namespace mir {
namespace action {

#ifdef ATLAS_HAVE_TRANS

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

static void createCoefficients(caching::LegendreCache& cache,
                               const std::string& key,
                               size_t truncation,
                               const atlas::grid::Grid &grid,
                               context::Context& ctx) {
    eckit::AutoTiming timing(ctx.statistics().timer_, ctx.statistics().createCoeffTiming_);

    struct Trans_t tmp_trans;
    fillTrans(tmp_trans, truncation, grid);

    eckit::PathName tmp = cache.stage(key);
    ASSERT(trans_set_write(&tmp_trans, tmp.asString().c_str()) == 0);
    ASSERT(trans_setup(&tmp_trans) == 0); // This will create the cache

    ASSERT(cache.commit(key, tmp));

    trans_delete(&tmp_trans);
}

static void transform(
    const std::string& key,
    const param::MIRParametrisation &parametrisation,
    size_t truncation,
    const std::vector<double> &input, std::vector<double> &output,
    const atlas::grid::Grid &grid,
    context::Context& ctx) {


    // Warning: we keep the coefficient in memory for all the resolution used
    if (trans_handles.find(key) == trans_handles.end()) {
        // std::cout << "Creating a new TRANS handle for " << key << std::endl;

        eckit::AutoTiming timing(ctx.statistics().timer_, ctx.statistics().coefficientTiming_);

        caching::LegendreCache cache;
        eckit::PathName path;

        // If not in cache

        if (!cache.get(key, path)) {

            eckit::Log::info() << "Coefficient cache file " << cache.entry(key) << " does not exist" << std::endl;


            std::ostringstream oss;
            oss << cache.entry(key) << ".lock";

            eckit::PathName lockFile(oss.str());

            eckit::FileLock locker(lockFile);

            eckit::AutoLock<eckit::FileLock> lock(locker);

            // Some
            if (!cache.get(key, path)) {
                eckit::Log::info() << "Creating coefficient cache file " << cache.entry(key) << std::endl;
                createCoefficients(cache, key, truncation, grid, ctx);
            }
            else {
                eckit::Log::info() << "Coefficient cache file " << cache.entry(key) << " created by another process" << std::endl;
            }

            ASSERT(cache.get(key, path));

        }

        {

            size_t before = eckit::Malloc::allocated();

            eckit::AutoTiming timing(ctx.statistics().timer_, ctx.statistics().loadCoeffTiming_);

            eckit::Timer timer("Loading coefficients");

            TransCache &tc = trans_handles[key];

            struct Trans_t &trans = tc.trans_;
            fillTrans(trans, truncation, grid);

            tc.inited_ = true;
            tc.loader_ = caching::LegendreLoaderFactory::build(parametrisation, path);
            // std::cout << "LegendreLoader " << *tc.loader_ << std::endl;

            ASSERT(trans_set_cache(&trans, tc.loader_->address(), tc.loader_->size()) == 0);

            ASSERT(trans_setup(&trans) == 0);

            size_t after = eckit::Malloc::allocated();

            trans_handles.footprint(key, after - before);

        }

    }

    eckit::AutoTiming timing(ctx.statistics().timer_, ctx.statistics().sh2gridTiming_);
    eckit::Timer timer("SH2GRID");


    TransCache &tc = trans_handles[key];
    struct Trans_t &trans = tc.trans_;

    // Initialise grid ===============================================



    ASSERT(trans.myproc == 1);

    ASSERT(trans.nspec2g == (int) input.size());

    int number_of_fields = 1; // number of fields

    std::vector<int> nfrom(number_of_fields, 1); // processors responsible for distributing each field
    std::vector<double> rspec(number_of_fields * trans.nspec2 );

    //==============================================================================

    struct DistSpec_t distspec = new_distspec(&trans);
    distspec.nfrom  = &nfrom[0];
    distspec.rspecg = &input[0];
    distspec.rspec  = &rspec[0];
    distspec.nfld   = number_of_fields;
    ASSERT(trans_distspec(&distspec) == 0);


    // Transform sp to gp fields

    std::vector<double> rgp (number_of_fields * trans.ngptot);

    struct InvTrans_t invtrans = new_invtrans(&trans);
    invtrans.nscalar   = number_of_fields;
    invtrans.rspscalar = &rspec[0];
    invtrans.rgp       = &rgp[0];
    ASSERT(trans_invtrans(&invtrans) == 0);


    // Gather all gridpoint fields

    output.resize(number_of_fields * trans.ngptotg);

    std::vector<int> nto (number_of_fields, 1);


    struct GathGrid_t gathgrid = new_gathgrid(&trans);
    gathgrid.rgp  = &rgp[0];
    gathgrid.rgpg = &output[0];
    gathgrid.nfld = number_of_fields;
    gathgrid.nto  = &nto[0];
    ASSERT(trans_gathgrid(&gathgrid) == 0);


    // trans_delete(&trans);
}
#endif


static void transform(const param::MIRParametrisation &parametrisation, size_t truncation,
                      const std::vector<double> &input, std::vector<double> &output,
                      const atlas::grid::Grid &grid,
                      context::Context& ctx) {
#ifdef ATLAS_HAVE_TRANS

    eckit::AutoLock<eckit::Mutex> lock(amutex); // To protect trans_handles

    static TransInitor initor; // Will init trans if needed

    std::ostringstream os;

    os << "T" << truncation << ":" << grid.uniqueId();
    std::string key(os.str());

    try {
        transform(key, parametrisation, truncation, input, output, grid, ctx);
    } catch (std::exception& e) {
        eckit::Log::error() << "Error while running SH2GRID: " << e.what() << std::endl;
        trans_handles.erase(key);
        throw;
    }

#else
    throw eckit::SeriousBug("Spherical harmonics transforms are not supported."
                            " Please recompile ATLAS with TRANS support enabled.");
#endif
}


Sh2GriddedTransform::Sh2GriddedTransform(const param::MIRParametrisation &parametrisation):
    Action(parametrisation) {
}


Sh2GriddedTransform::~Sh2GriddedTransform() {
}


void Sh2GriddedTransform::execute(context::Context & ctx) const {
    // ASSERT(field.dimensions() == 1); // For now
#ifdef ATLAS_HAVE_TRANS
    // Make sure another thread to no evict anything from the cache while we are using it
    InMemoryCacheUser<TransCache> use(trans_handles, ctx.statistics().transHandleCache_);
#endif

    data::MIRField& field = ctx.field();


    repres::RepresentationHandle out(outputRepresentation());

    // std::cout << *out << std::endl;

    // TODO: Transform all the fields together
    for (size_t i = 0; i < field.dimensions(); i++) {

        const std::vector<double> &values = field.values(i);
        std::vector<double> result;

        eckit::ScopedPtr<atlas::grid::Grid> grid(out->atlasGrid());
        transform(parametrisation_,
                  field.representation()->truncation(),
                  values,
                  result,
                  *grid,
                  ctx);

        field.update(result, i);

    }

    field.representation(out);
}


}  // namespace action
}  // namespace mir

