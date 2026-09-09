// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/netcdf/SimpleOutputVariable.h"

#include <ostream>

namespace mir::netcdf {


SimpleOutputVariable::SimpleOutputVariable(Dataset& owner, const std::string& name,
                                           const std::vector<Dimension*>& dimensions) :
    OutputVariable(owner, name, dimensions) {}

SimpleOutputVariable::~SimpleOutputVariable() = default;

void SimpleOutputVariable::print(std::ostream& out) const {
    out << "SimpleOutputVariable[name=" << name_ << "]";
}

}  // namespace mir::netcdf
