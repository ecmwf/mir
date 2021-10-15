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


#include "mir/netcdf/CoordinateInputVariable.h"

#include <ostream>

#include "mir/netcdf/CoordinateOutputVariable.h"
#include "mir/netcdf/ScalarCoordinateInputVariable.h"


namespace mir {
namespace netcdf {


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


}  // namespace netcdf
}  // namespace mir
