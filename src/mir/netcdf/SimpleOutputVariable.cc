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


#include "mir/netcdf/SimpleOutputVariable.h"

#include <ostream>

namespace mir {
namespace netcdf {


SimpleOutputVariable::SimpleOutputVariable(Dataset& owner, const std::string& name,
                                           const std::vector<Dimension*>& dimensions) :
    OutputVariable(owner, name, dimensions) {}

SimpleOutputVariable::~SimpleOutputVariable() = default;

void SimpleOutputVariable::print(std::ostream& out) const {
    out << "SimpleOutputVariable[name=" << name_ << "]";
}

}  // namespace netcdf
}  // namespace mir
