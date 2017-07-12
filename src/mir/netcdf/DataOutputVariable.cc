/*
 * (C) Copyright 1996-2013 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

// Baudouin Raoult - ECMWF Jan 2015

#include "mir/netcdf/DataOutputVariable.h"

#include "mir/netcdf/Attribute.h"
#include "mir/netcdf/MergeDataStep.h"
#include "mir/netcdf/MergePlan.h"
#include "mir/netcdf/Exceptions.h"

namespace mir {
namespace netcdf {

DataOutputVariable::DataOutputVariable(Dataset &owner,
                                       const std::string &name,
                                       const std::vector<Dimension *> &dimensions):
    OutputVariable(owner, name, dimensions)
{
}

DataOutputVariable::~DataOutputVariable() {

}

void DataOutputVariable::print(std::ostream &out) const {
    out << "DataOutputVariable[name=" << name_ << ",nc=" << ncname() << "]";
}

void DataOutputVariable::merge(const Variable &other, MergePlan &plan)
{
    Variable::merge(other, plan);
    plan.add(new MergeDataStep(*this, other));
}

const std::string &DataOutputVariable::ncname() const {
    auto j = attributes_.find("standard_name");
    if (j != attributes_.end()) {
        ncname_ = (*j).second->asString();
        return ncname_;
    }
    return name();
}


void DataOutputVariable::collectField(std::vector<Field *>&) const {
    NOTIMP;
}

}
}
