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


#include "mir/netcdf/MergeDataStep.h"

#include <ostream>

#include "mir/netcdf/Dimension.h"
#include "mir/netcdf/Matrix.h"
#include "mir/netcdf/Type.h"
#include "mir/netcdf/Variable.h"


namespace mir::netcdf {


MergeDataStep::MergeDataStep(Variable& out, const Variable& in) : out_(out), in_(in) {}


int MergeDataStep::rank() const {
    return 5;
}


void MergeDataStep::print(std::ostream& out) const {
    out << "MergeDataStep[" << out_ << " & " << in_ << "]";
}


void MergeDataStep::execute(MergePlan& /*plan*/) {
#if 0
    if (!out_.mustMerge()) {

        Type &type1 = in_.matrix()->type();
        Type &type2 = out_.matrix()->type();

        if (type1 != type2) {
            Type &common = Type::lookup(type1, type2);
            out_.matrix()->type(common);
        }

        return;
    }

    // Resize dimensions

    size_t i = 0;
    for (auto j = out_.dimensions().begin(); j != out_.dimensions().end(); ++j, ++i) {
        (*j)->grow(out_.cube().dimensions(i));
    }

    // Save matrix
    // Matrix *m = out_.matrix()->mergeData(in_.matrix(), out_.cube().count());
    // out_.setMatrix(m);
#endif
}


}  // namespace mir::netcdf
