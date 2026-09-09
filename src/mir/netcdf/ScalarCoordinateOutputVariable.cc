// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/netcdf/ScalarCoordinateOutputVariable.h"

#include "mir/netcdf/Matrix.h"
#include "mir/netcdf/Type.h"

namespace mir::netcdf {

ScalarCoordinateOutputVariable::ScalarCoordinateOutputVariable(Dataset& owner, const std::string& name,
                                                               const std::vector<Dimension*>& dimensions) :
    OutputVariable(owner, name, dimensions) {}

ScalarCoordinateOutputVariable::~ScalarCoordinateOutputVariable() = default;

void ScalarCoordinateOutputVariable::print(std::ostream& out) const {
    out << "ScalarCoordinateOutputVariable[name=" << name_ << "]";
}


void ScalarCoordinateOutputVariable::merge(const Variable& other, MergePlan& plan) {
    Variable::merge(other, plan);

    Type& type1 = matrix()->type();
    Type& type2 = other.matrix()->type();

    Type& common = Type::lookup(type1, type2);
    if (common.coordinateOutputVariableMerge(*this, other, plan)) {
        // Merge done
    }
    else {
        if (type1 != type2) {
            // Upgrade type
            matrix()->type(common);
        }
        else {
            // NO CHANGE
        }
    }
}

}  // namespace mir::netcdf
