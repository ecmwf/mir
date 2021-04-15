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


#include "mir/netcdf/CellMethodInputVariable.h"
#include "mir/netcdf/CellMethodOutputVariable.h"

#include <ostream>

namespace mir {
namespace netcdf {

CellMethodInputVariable::CellMethodInputVariable(Dataset& owner, const std::string& name, int id,
                                                 const std::vector<Dimension*>& dimensions) :
    InputVariable(owner, name, id, dimensions) {}

CellMethodInputVariable::~CellMethodInputVariable() = default;

Variable* CellMethodInputVariable::makeOutputVariable(Dataset& owner, const std::string& name,
                                                      const std::vector<Dimension*>& dimensions) const {
    return new CellMethodOutputVariable(owner, name, dimensions);
}

void CellMethodInputVariable::print(std::ostream& out) const {
    out << "CellMethodInputVariable[name=" << name_ << "]";
}

const char* CellMethodInputVariable::kind() const {
    return "cell method";
}


}  // namespace netcdf
}  // namespace mir
