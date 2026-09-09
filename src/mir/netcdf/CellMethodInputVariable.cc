// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/netcdf/CellMethodInputVariable.h"

#include <ostream>

#include "mir/netcdf/CellMethodOutputVariable.h"


namespace mir::netcdf {

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


}  // namespace mir::netcdf
