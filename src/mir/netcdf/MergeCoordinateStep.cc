// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/netcdf/MergeCoordinateStep.h"

#include <ostream>

#include "mir/netcdf/Dimension.h"
#include "mir/netcdf/Matrix.h"
#include "mir/netcdf/Type.h"
#include "mir/netcdf/Variable.h"


namespace mir::netcdf {


MergeCoordinateStep::MergeCoordinateStep(Variable& out, const Variable& in) : out_(out), in_(in) {}


int MergeCoordinateStep::rank() const {
    return 4;
}


void MergeCoordinateStep::print(std::ostream& out) const {
    out << "MergeCoordinateStep[" << out_ << " & " << in_ << "]";
}


void MergeCoordinateStep::execute(MergePlan& /*plan*/) {
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

    Log::info() << "MergeCoordinateStep::execute() " << out_ << ", Output variable cube:" << out_.cube() << std::endl;

    // Resize dimensions

    size_t i = 0;
    for (auto j = out_.dimensions().begin(); j != out_.dimensions().end(); ++j, ++i) {
        (*j)->grow(out_.cube().dimensions(i));
    }

    // Save matrix
    // Matrix *m = out_.matrix()->mergeCoordinates(in_.matrix(), out_.cube().count());
    // out_.setMatrix(m);
#endif
}


}  // namespace mir::netcdf
