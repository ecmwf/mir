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
#include "mir/data/MIRField.h"
#include "mir/param/MIRParametrisation.h"


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


    // transform
    struct InvTrans_t invtrans = new_invtrans(&trans);
    invtrans.rgp     = output.data();
    invtrans.nvordiv = 1;
    invtrans.rspvor  = field.values(0).data();
    invtrans.rspdiv  = field.values(1).data();
    ASSERT(trans_invtrans(&invtrans) == 0);


    // set u/v field values
    long id_u = 131;
    long id_v = 132;
    parametrisation_.get("transform.u", id_u);
    parametrisation_.get("transform.v", id_v);

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

