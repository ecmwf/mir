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


#include "mir/action/transform/Sh2GriddedTransform.h"

#include <iostream>
#include <vector>

#include "atlas/atlas_config.h"
#include "atlas/Grid.h"
#include "atlas/grids/grids.h"

#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/log/Timer.h"
#include "eckit/utils/MD5.h"

#include "mir/data/MIRField.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/caching/LegendreCache.h"
#include "mir/caching/LegendreLoader.h"

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
    struct Trans_t trans_;
    mir::caching::LegendreLoader *loader_;
    TransCache(): loader_(0) {}
};

static eckit::Mutex amutex;
static std::map<std::string, TransCache> trans_handles;

#endif

namespace mir {
namespace action {


static void transform(const param::MIRParametrisation &parametrisation, size_t truncation,
                      const std::vector<double> &input, std::vector<double> &output, const atlas::Grid &grid) {
#ifdef ATLAS_HAVE_TRANS

    eckit::AutoLock<eckit::Mutex> lock(amutex); // To protect trans_handles

    static TransInitor initor; // Will init trans if needed

    const atlas::grids::ReducedGrid *reduced = dynamic_cast<const atlas::grids::ReducedGrid *>(&grid);

    if (!reduced) {
        throw eckit::SeriousBug("Spherical harmonics transforms only supports SH to ReducedGG/RegularGG/RegularLL.");
    }

    const atlas::grids::LonLatGrid *latlon = dynamic_cast<const atlas::grids::LonLatGrid *>(&grid);

    eckit::StrStream os;


    os << "T" << truncation << ":" << grid.unique_id() << eckit::StrStream::ends;
    std::string key(os);


    // Warning: we keep the coefficient in memory for all the resolution used
    if (trans_handles.find(key) == trans_handles.end()) {
        eckit::Log::info() << "Creating a new TRANS handle for " << key << std::endl;

        TransCache &tc = trans_handles[key];
        struct Trans_t &trans = tc.trans_;

        ASSERT(trans_new(&trans) == 0);

        ASSERT(trans_set_trunc(&trans, truncation) == 0);

        if (latlon) {
            ASSERT(trans_set_resol_lonlat(&trans, latlon->nlon(), latlon->nlat()) == 0);
        } else {
            const std::vector<int> &points_per_latitudes = reduced->npts_per_lat();
            ASSERT(trans_set_resol(&trans, points_per_latitudes.size(), &points_per_latitudes[0]) == 0);
        }

        //
        caching::LegendreCache cache;
        eckit::PathName path;
        if (!cache.get(key, path)) {
            eckit::Timer timer("Caching coefficients");
            eckit::Log::info() << "LegendreCache " << key << " does not exists" << std::endl;
            eckit::PathName tmp = cache.stage(key);
            ASSERT( trans_set_write(&trans, tmp.asString().c_str())  == 0);
            ASSERT(trans_setup(&trans) == 0);

            ASSERT(cache.commit(key, tmp));
        } else {

            tc.loader_ = caching::LegendreLoaderFactory::build(parametrisation, path);
            eckit::Log::info() << "LegendreLoader " << *tc.loader_ << std::endl;

            ASSERT(trans_set_cache(&trans, tc.loader_->address(), tc.loader_->size()) == 0);

            ASSERT(trans_setup(&trans) == 0);
        }
    }

    TransCache &tc = trans_handles[key];
    struct Trans_t &trans = tc.trans_;

    // Initialise grid ===============================================



    ASSERT(trans.myproc == 1);

    ASSERT(trans.nspec2g == input.size());

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

#else
    throw eckit::SeriousBug("Spherical harmonics transforms are not supported."
                            " Please recompile ATLAS was not compiled with TRANS support.");
#endif
}




Sh2GriddedTransform::Sh2GriddedTransform(const param::MIRParametrisation &parametrisation):
    Action(parametrisation) {
}


Sh2GriddedTransform::~Sh2GriddedTransform() {
}


void Sh2GriddedTransform::execute(data::MIRField &field) const {
    // ASSERT(field.dimensions() == 1); // For now

    repres::Representation *out = outputRepresentation(field.representation());

    // TODO: Transform all the fields together
    for (size_t i = 0; i < field.dimensions(); i++) {


        const std::vector<double> &values = field.values(i);
        std::vector<double> result;

        const repres::Representation *in = field.representation();

        try {
            std::auto_ptr<atlas::Grid> grid(out->atlasGrid());
            transform(parametrisation_, in->truncation(), values, result, *grid);
        } catch (...) {
            delete out;
            throw;
        }


        field.values(result, i);

    }

    field.representation(out);
}


}  // namespace action
}  // namespace mir

