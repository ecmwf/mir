// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/netcdf/CellMethodOutputVariable.h"

#include <ostream>

#include "mir/netcdf/MergeDataStep.h"
#include "mir/netcdf/MergePlan.h"


namespace mir::netcdf {

CellMethodOutputVariable::CellMethodOutputVariable(Dataset& owner, const std::string& name,
                                                   const std::vector<Dimension*>& dimensions) :
    OutputVariable(owner, name, dimensions) {}


CellMethodOutputVariable::~CellMethodOutputVariable() = default;


void CellMethodOutputVariable::print(std::ostream& out) const {
    out << "CellMethodOutputVariable[name=" << name_ << "]";
}


void CellMethodOutputVariable::merge(const Variable& other, MergePlan& plan) {
    Variable::merge(other, plan);
    plan.add(new MergeDataStep(*this, other));
}


}  // namespace mir::netcdf
