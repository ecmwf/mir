// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/netcdf/InputAttribute.h"

#include <ostream>

#include "mir/netcdf/Endowed.h"
#include "mir/netcdf/OutputAttribute.h"
#include "mir/netcdf/Value.h"


namespace mir::netcdf {


InputAttribute::InputAttribute(Endowed& owner, const std::string& name, Value* value) : Attribute(owner, name, value) {}


InputAttribute::~InputAttribute() = default;


void InputAttribute::clone(Endowed& owner) const {
    owner.add(new OutputAttribute(owner, name_, value_->clone()));
}


void InputAttribute::print(std::ostream& out) const {
    out << "InputAttribute[name=" << name_ << "]";
}


}  // namespace mir::netcdf
