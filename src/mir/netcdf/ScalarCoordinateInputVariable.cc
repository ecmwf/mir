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


#include "mir/netcdf/ScalarCoordinateInputVariable.h"

#include "mir/netcdf/Dataset.h"
#include "mir/netcdf/ScalarCoordinateOutputVariable.h"
#include "mir/netcdf/VirtualInputDimension.h"

#include <ostream>

namespace mir {
namespace netcdf {


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


}  // namespace netcdf
}  // namespace mir
