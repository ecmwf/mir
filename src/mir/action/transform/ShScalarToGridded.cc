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


#include "mir/action/transform/ShScalarToGridded.h"

#include <vector>
#include "eckit/exception/Exceptions.h"
#include "mir/data/MIRField.h"


namespace mir {
namespace action {
namespace transform {


ShScalarToGridded::ShScalarToGridded(const param::MIRParametrisation& parametrisation):
    ShToGridded(parametrisation) {
}


ShScalarToGridded::~ShScalarToGridded() {
}


void ShScalarToGridded::sh2grid(struct Trans_t& trans, data::MIRField& field) const {
    size_t number_of_fields = field.dimensions();
    ASSERT(number_of_fields > 0);
    ASSERT(trans.myproc == 1);
    ASSERT(trans.nspec2g == int(field.values(0).size()));


    // set input & output working area (avoid copies if transforming one field only)
    std::vector<double> output(number_of_fields * size_t(trans.ngptotg));
    std::vector<double> input;
    if (number_of_fields > 1) {
        long size = long(field.values(0).size());
        input.resize(number_of_fields * size_t(size));

        // spectral coefficients are "interlaced"
        for (size_t i = 0; i < number_of_fields; i++) {
            const std::vector<double>& values = field.values(i);
            ASSERT(int(values.size()) == trans.nspec2g);

            for (size_t j = 0; j < size_t(size); ++j) {
                input[ j * number_of_fields + i ] = values[j];
            }
        }
    }

    std::vector<int>    nfrom (number_of_fields, 1); // processors responsible for distributing each field
    std::vector<int>    nto   (number_of_fields, 1);
    std::vector<double> rspec (number_of_fields * trans.nspec2);
    std::vector<double> rgp   (number_of_fields * trans.ngptot);
    int nfld = int(number_of_fields);


    // distribute
    struct DistSpec_t distspec = new_distspec(&trans);
    distspec.nfrom  = nfrom.data();
    distspec.rspecg = number_of_fields > 1 ? input.data() : field.values(0).data();
    distspec.rspec  = rspec.data();
    distspec.nfld   = nfld;
    ASSERT(trans_distspec(&distspec) == 0);


    // transform
    struct InvTrans_t invtrans = new_invtrans(&trans);
    invtrans.nscalar   = nfld;
    invtrans.rspscalar = rspec.data();
    invtrans.rgp       = rgp.data();
    ASSERT(trans_invtrans(&invtrans) == 0);


    // gather
    struct GathGrid_t gathgrid = new_gathgrid(&trans);
    gathgrid.rgp  = rgp.data();
    gathgrid.rgpg = output.data();
    gathgrid.nfld = nfld;
    gathgrid.nto  = nto.data();
    ASSERT(trans_gathgrid(&gathgrid) == 0);


    // set field values (again, avoid copies for one field only)
    if (number_of_fields == 1) {
//        output.resize(size_t(trans.ngptotg));
        field.update(output, 0);
    } else {
        std::vector<double>::const_iterator here = output.begin();
        for (size_t i = 0; i < number_of_fields; i++) {
            std::vector<double> output_field(here, here + trans.ngptotg);

            field.update(output_field, i);
            here += trans.ngptotg;
        }
    }

}


}  // namespace transform
}  // namespace action
}  // namespace mir

