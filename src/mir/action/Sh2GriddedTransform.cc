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


#include "mir/action/Sh2GriddedTransform.h"

#include <iostream>

#include "atlas/Grid.h"
#include "atlas/grids/grids.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/log/Timer.h"
#include "eckit/parser/Tokenizer.h"
#include "eckit/utils/Translator.h"
#include "mir/data/MIRField.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"

#include "atlas/atlas_config.h"

#ifdef ATLAS_HAVE_TRANS
#include "transi/trans.h"
#endif

namespace mir {
namespace action {

static void transform(size_t truncation, const std::vector<double> &input, std::vector<double> &output, const atlas::Grid& grid) {
#ifdef ATLAS_HAVE_TRANS

    const atlas::grids::ReducedGaussianGrid* rgg = dynamic_cast<const atlas::grids::ReducedGaussianGrid*>(&grid);
    if(!rgg) {
        NOTIMP;
    }


    std::vector<int> nloen(rgg->npts_per_lat());
    for(int i = 0; i < nloen.size(); i++) {
        std::cout << i << " " << nloen[i] << std::endl;
    }

    struct Trans_t trans = new_trans();

    trans.ndgl  = nloen.size();
    trans.nloen = &nloen[0];

    long maxtr = 0; // p["MaxTruncation"];

    trans.nsmax = maxtr ? maxtr : (2 * trans.ndgl - 1) / 2; // assumption: linear grid


    // register resolution in trans library
    {
        eckit::Timer t("setup");
        trans_setup( &trans );
    }

    ASSERT(trans.myproc == 1);

    int number_of_fields = 1; // number of fields

    std::vector<int> nfrom(number_of_fields, 1); // processors responsible for distributing each field

    std::vector<double> rspec ( number_of_fields * trans.nspec2  );

    struct DistSpec_t distspec = new_distspec(&trans);
    distspec.nfrom  = &nfrom[0];
    distspec.rspecg = const_cast<double *>(&input[0]);
    distspec.rspec  = &rspec[0];
    distspec.nfld   = number_of_fields;

    {
        eckit::Timer t("distribute");
        trans_distspec(&distspec);
    }

    // Transform sp to gp fields

    std::vector<double> rgp ( number_of_fields * trans.ngptot );

    struct InvTrans_t invtrans = new_invtrans(&trans);
    invtrans.nscalar   = number_of_fields;
    invtrans.rspscalar = &rspec[0];
    invtrans.rgp       = &rgp[0];

    {
        eckit::Timer t("transform");
        trans_invtrans(&invtrans);
    }

    // Gather all gridpoint fields

    output.resize( number_of_fields * trans.ngptotg );

    std::vector<int> nto ( number_of_fields , 1 );
    // for ( int jfld = 0; jfld < number_of_fields; ++jfld )
    //     nto[jfld] = 1;

    struct GathGrid_t gathgrid = new_gathgrid(&trans);
    gathgrid.rgp  = &rgp[0];
    gathgrid.rgpg = &output[0];
    gathgrid.nfld = number_of_fields;
    gathgrid.nto  = &nto[0];

    {
        eckit::Timer t("gather");
        trans_gathgrid(&gathgrid);
    }


    // FIXME: double memory free happening

    // trans_delete(&trans);
    // trans_finalize();
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


void Sh2GriddedTransform::print(std::ostream &out) const {
    out << "Sh2GriddedTransform[]";
}


void Sh2GriddedTransform::execute(data::MIRField &field) const {
    const std::vector<double> &values = field.values();
    std::vector<double> result;

    const repres::Representation *in = field.representation();
    // repres::Representation *repres = representation->truncate(truncation_, values, result);

    // if (repres) { // NULL if nothing happend
    //     field.representation(repres);
    //     field.values(result);
    // }

    repres::Representation *out = outputRepresentation(field.representation());

    try {
        std::auto_ptr<atlas::Grid> grid(out->atlasGrid());
        transform(in->truncation(), values, result, *grid);
    } catch (...) {
        delete out;
        throw;
    }

    for(size_t i = 0; i < result.size(); ++i) {
        std::cout << result[i] << std::endl;
        if(i > 10) {
            break;
        }
    }
    // field.representation(out);
}


}  // namespace action
}  // namespace mir

