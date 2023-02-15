/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/netcdf/UpdateCoordinateStep.h"

#include <ostream>

#include "mir/netcdf/Dataset.h"
#include "mir/netcdf/Exceptions.h"
#include "mir/netcdf/MergeCoordinateStep.h"
#include "mir/netcdf/MergePlan.h"
#include "mir/netcdf/ReshapeVariableStep.h"
#include "mir/netcdf/Variable.h"


namespace mir::netcdf {

UpdateCoordinateStep::UpdateCoordinateStep(Variable& out, const Variable& in) : out_(out), in_(in) {}

int UpdateCoordinateStep::rank() const {
    return 1;
}

void UpdateCoordinateStep::print(std::ostream& out) const {
    out << "UpdateCoordinateStep[" << in_ << " & " << out_ << "]";
}

void UpdateCoordinateStep::execute(MergePlan& /*plan*/) {
#if 0
    size_t growth_;

    const std::vector<Dimension *> &dims = out_.dimensions();
    ASSERT(dims.size() == 1);

    Log::info() << *this << std::endl;

    std::vector<Variable *> v = plan.field().variablesForDimension(*dims[0]);
    for (std::vector<Variable *>::iterator j = v.begin(); j != v.end(); ++j) {
        Log::info() << "Affects: " << **j << std::endl;
        (*j)->mustMerge(true);
        plan.add(new ReshapeVariableStep(**j, *dims[0], growth_));
    }

    plan.add(new MergeCoordinateStep(out_, in_));
#endif
}

}  // namespace mir::netcdf
