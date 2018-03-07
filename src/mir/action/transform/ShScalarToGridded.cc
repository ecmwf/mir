/*
 * (C) Copyright 1996- ECMWF.
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
#include "eckit/log/ResourceUsage.h"
#include "mir/config/LibMir.h"
#include "mir/data/MIRField.h"
#include "mir/repres/Representation.h"


namespace mir {
namespace action {
namespace transform {


ShScalarToGridded::ShScalarToGridded(const param::MIRParametrisation& parametrisation):
    ShToGridded(parametrisation) {
}


ShScalarToGridded::~ShScalarToGridded() {
}


void ShScalarToGridded::sh2grid(data::MIRField& field, atlas_trans_t& trans, const atlas::Grid& grid) const {
    eckit::Timer timer("ShScalarToGridded::sh2grid", eckit::Log::debug<mir::LibMir>());

    size_t number_of_fields = field.dimensions();
    ASSERT(number_of_fields == 1);

    // do inverse transform and set gridded values
    std::vector<double> output(grid.size());
    trans.invtrans(1, field.values(0).data(), output.data(), atlas::option::global());

    field.update(output, 0);
}


}  // namespace transform
}  // namespace action
}  // namespace mir

