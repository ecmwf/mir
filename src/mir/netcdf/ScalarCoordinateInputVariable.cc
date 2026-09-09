// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/netcdf/ScalarCoordinateInputVariable.h"

#include <ostream>

#include "mir/netcdf/Dataset.h"
#include "mir/netcdf/ScalarCoordinateOutputVariable.h"
#include "mir/netcdf/VirtualInputDimension.h"


namespace mir::netcdf {


ScalarCoordinateInputVariable::ScalarCoordinateInputVariable(Dataset& owner, const std::string& name, int id,
                                                             const std::vector<Dimension*>& dimensions) :
    InputVariable(owner, name, id, dimensions) {}


ScalarCoordinateInputVariable::~ScalarCoordinateInputVariable() = default;


Variable* ScalarCoordinateInputVariable::makeOutputVariable(Dataset& owner, const std::string& name,
                                                            const std::vector<Dimension*>& dimensions) const {
    return new ScalarCoordinateOutputVariable(owner, name, dimensions);
}


Variable* ScalarCoordinateInputVariable::makeScalarCoordinateVariable() {
    return this;
}


Variable* ScalarCoordinateInputVariable::makeCoordinateVariable() {
    return this;
}


void ScalarCoordinateInputVariable::print(std::ostream& out) const {
    out << "ScalarCoordinateInputVariable[name=" << name_ << "]";
}


Dimension* ScalarCoordinateInputVariable::getVirtualDimension() {
    if (dimensions_.empty()) {
        dimensions_.emplace_back(new VirtualInputDimension(dataset_, name_));
        dataset_.add(dimensions_.back());
    }
    return dimensions_.front();
}


}  // namespace mir::netcdf
