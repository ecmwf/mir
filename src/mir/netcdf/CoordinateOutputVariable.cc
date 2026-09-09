// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/netcdf/CoordinateOutputVariable.h"

#include "mir/netcdf/Matrix.h"
#include "mir/netcdf/Type.h"


namespace mir::netcdf {


CoordinateOutputVariable::CoordinateOutputVariable(Dataset& owner, const std::string& name,
                                                   const std::vector<Dimension*>& dimensions) :
    OutputVariable(owner, name, dimensions) {}


CoordinateOutputVariable::~CoordinateOutputVariable() = default;


void CoordinateOutputVariable::print(std::ostream& out) const {
    out << "CoordinateOutputVariable[name=" << name_ << "]";
}


void CoordinateOutputVariable::merge(const Variable& other, MergePlan& plan) {

    Variable::merge(other, plan);

    Type& type1 = matrix()->type();
    Type& type2 = other.matrix()->type();

    Type& common = Type::lookup(type1, type2);
    if (common.coordinateOutputVariableMerge(*this, other, plan)) {
        // Merge done
    }
    else {
        if (type1 != type2) {
            // Type change
            matrix()->type(common);
        }
        else {
            // No change
        }
    }
}


}  // namespace mir::netcdf
