// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/netcdf/DataOutputVariable.h"

#include "mir/netcdf/Attribute.h"
#include "mir/netcdf/Exceptions.h"
#include "mir/netcdf/MergeDataStep.h"
#include "mir/netcdf/MergePlan.h"

namespace mir::netcdf {

DataOutputVariable::DataOutputVariable(Dataset& owner, const std::string& name,
                                       const std::vector<Dimension*>& dimensions) :
    OutputVariable(owner, name, dimensions) {}

DataOutputVariable::~DataOutputVariable() = default;

void DataOutputVariable::print(std::ostream& out) const {
    out << "DataOutputVariable[name=" << name_ << ",nc=" << ncname() << "]";
}

void DataOutputVariable::merge(const Variable& other, MergePlan& plan) {
    Variable::merge(other, plan);
    plan.add(new MergeDataStep(*this, other));
}

const std::string& DataOutputVariable::ncname() const {
    auto j = attributes_.find("standard_name");
    if (j != attributes_.end()) {
        ncname_ = j->second->asString();
        return ncname_;
    }
    return name();
}


void DataOutputVariable::collectField(std::vector<Field*>& /*unused*/) const {
    NOTIMP;
}

}  // namespace mir::netcdf
