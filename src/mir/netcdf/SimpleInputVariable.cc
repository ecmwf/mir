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

#include "mir/netcdf/SimpleInputVariable.h"

#include "mir/netcdf/CellMethodInputVariable.h"
#include "mir/netcdf/CoordinateInputVariable.h"
#include "mir/netcdf/DataInputVariable.h"
#include "mir/netcdf/Exceptions.h"
#include "mir/netcdf/SimpleOutputVariable.h"

namespace mir {
namespace netcdf {

SimpleInputVariable::SimpleInputVariable(Dataset &owner, const std::string &name, int id, const std::vector<Dimension *> &dimensions):
    InputVariable(owner, name, id, dimensions)
{
}

SimpleInputVariable::~SimpleInputVariable() {

}

Variable *SimpleInputVariable::makeDataVariable() {
    Variable *v = new DataInputVariable(dataset_, name_, id_, dimensions_);
    v->copyAttributes(*this);
    v->setMatrix(matrix_);
    return v;
}

Variable *SimpleInputVariable::makeCoordinateVariable() {
    Variable *v = new CoordinateInputVariable(dataset_, name_, id_, dimensions_);
    v->copyAttributes(*this);
    v->setMatrix(matrix_);
    return v;
}

Variable *SimpleInputVariable::makeCellMethodVariable() {
    Variable *v = new CellMethodInputVariable(dataset_, name_, id_, dimensions_);
    v->copyAttributes(*this);
    v->setMatrix(matrix_);
    return v;
}

Variable *SimpleInputVariable::makeOutputVariable(Dataset &owner, const std::string &name, const std::vector<Dimension *> &dimensions) const {
    return new SimpleOutputVariable(owner, name, dimensions);
}

void SimpleInputVariable::print(std::ostream &out) const {
    out << "SimpleInputVariable[name=" << name_ << "]";
}

void SimpleInputVariable::validate() const {
    eckit::Log::error() << "Variable '" << name_ << "' is not data, coordinate or cell method." << std::endl;
    // throw MergeError(std::string("Variable ") + name_ + " is not data, coordinate or cell method.");
}

}
}
