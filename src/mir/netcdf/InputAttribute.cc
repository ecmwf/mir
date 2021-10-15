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


#include "mir/netcdf/InputAttribute.h"

#include <ostream>

#include "mir/netcdf/Endowed.h"
#include "mir/netcdf/OutputAttribute.h"
#include "mir/netcdf/Value.h"


namespace mir {
namespace netcdf {


InputAttribute::InputAttribute(Endowed& owner, const std::string& name, Value* value) : Attribute(owner, name, value) {}


InputAttribute::~InputAttribute() = default;


void InputAttribute::clone(Endowed& owner) const {
    owner.add(new OutputAttribute(owner, name_, value_->clone()));
}


void InputAttribute::print(std::ostream& out) const {
    out << "InputAttribute[name=" << name_ << "]";
}


}  // namespace netcdf
}  // namespace mir
