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


#include "mir/netcdf/CellMethodOutputVariable.h"

#include <ostream>

#include "mir/netcdf/MergeDataStep.h"
#include "mir/netcdf/MergePlan.h"


namespace mir {
namespace netcdf {

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


}  // namespace netcdf
}  // namespace mir
