// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/netcdf/CoordinateInputVariable.h"

#include <ostream>

#include "mir/netcdf/CoordinateOutputVariable.h"
#include "mir/netcdf/ScalarCoordinateInputVariable.h"


namespace mir::netcdf {


CoordinateInputVariable::CoordinateInputVariable(Dataset& owner, const std::string& name, int id,
                                                 const std::vector<Dimension*>& dimensions) :
    InputVariable(owner, name, id, dimensions) {}


CoordinateInputVariable::~CoordinateInputVariable() = default;


Variable* CoordinateInputVariable::makeOutputVariable(Dataset& owner, const std::string& name,
                                                      const std::vector<Dimension*>& dimensions) const {
    return new CoordinateOutputVariable(owner, name, dimensions);
}


Variable* CoordinateInputVariable::makeCoordinateVariable() {
    return this;
}


Variable* CoordinateInputVariable::makeScalarCoordinateVariable() {
    Variable* v = new ScalarCoordinateInputVariable(dataset_, name_, id_, dimensions_);
    v->copyAttributes(*this);
    v->setMatrix(matrix());
    return v;
}


void CoordinateInputVariable::print(std::ostream& out) const {
    out << "CoordinateInputVariable[name=" << name_ << "]";
}


const char* CoordinateInputVariable::kind() const {
    return "coordinate";
}


}  // namespace mir::netcdf
