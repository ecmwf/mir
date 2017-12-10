/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @date Feb 2017


#include "mir/action/transform/ShVodTouvGridded.h"

#include <vector>
#include "eckit/exception/Exceptions.h"
#include "mir/config/LibMir.h"
#include "mir/data/MIRField.h"
#include "mir/param/MIRParametrisation.h"
#include "eckit/log/ResourceUsage.h"
#include "mir/config/LibMir.h"


namespace mir {
namespace action {
namespace transform {


ShVodTouvGridded::ShVodTouvGridded(const param::MIRParametrisation& parametrisation):
    ShToGridded(parametrisation) {
}


ShVodTouvGridded::~ShVodTouvGridded() {
}


void ShVodTouvGridded::sh2grid(struct Trans_t& trans, data::MIRField& field) const {
    size_t number_of_fields = field.dimensions();
    ASSERT(number_of_fields == 2);
    ASSERT(trans.myproc == 1);
    ASSERT(trans.nspec2g == int(field.values(0).size()));


    // set output working area
    std::vector<double> output(number_of_fields * size_t(trans.ngptotg));


    std::vector<int>    nfrom (number_of_fields, 1); // processors responsible for distributing each field
    std::vector<int>    nto   (number_of_fields, 1);
    std::vector<double> rgp   (number_of_fields * size_t(trans.ngptot));
    std::vector<double> rspec_vo (size_t(trans.nspec2));
    std::vector<double> rspec_d  (size_t(trans.nspec2));

    {
        eckit::TraceResourceUsage<LibMir> usage("SH2GG ShVodTouvGridded");

        // distribute
        struct DistSpec_t distspec = new_distspec(&trans);
        distspec.nfrom  = nfrom.data();
        distspec.nfld   = 1;
        distspec.rspecg = field.values(0).data();
        distspec.rspec  = rspec_vo.data();
        ASSERT(trans_distspec(&distspec) == 0);

        distspec = new_distspec(&trans);
        distspec.nfrom  = nfrom.data();
        distspec.nfld   = 1;
        distspec.rspecg = field.values(1).data();
        distspec.rspec  = rspec_d.data();
        ASSERT(trans_distspec(&distspec) == 0);


        // transform
        struct InvTrans_t invtrans = new_invtrans(&trans);
        invtrans.nvordiv = 1;
        invtrans.rspvor  = rspec_vo.data();
        invtrans.rspdiv  = rspec_d.data();
        invtrans.rgp     = rgp.data();
        ASSERT(trans_invtrans(&invtrans) == 0);


        // gather
        struct GathGrid_t gathgrid = new_gathgrid(&trans);
        gathgrid.rgp  = rgp.data();
        gathgrid.rgpg = output.data();
        gathgrid.nfld = 2;
        gathgrid.nto  = nto.data();
        ASSERT(trans_gathgrid(&gathgrid) == 0);
    }

    // configure paramIds for u/v
    const eckit::Configuration& config = LibMir::instance().configuration();
    const long id_u = config.getLong("parameter-id-u", 131);
    const long id_v = config.getLong("parameter-id-v", 132);


    std::vector<double> result(output.begin(), output.begin() + trans.ngptotg);
    field.update(result, 0);
    field.metadata(0, "paramId", id_u);

    result.assign(output.begin() + trans.ngptotg, output.end());
    field.update(result, 1);
    field.metadata(1, "paramId", id_v);

}


}  // namespace transform
}  // namespace action
}  // namespace mir

