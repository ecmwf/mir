// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/netcdf/InputDimension.h"

#include <ostream>

#include "mir/netcdf/Dataset.h"
#include "mir/netcdf/OutputDimension.h"


namespace mir::netcdf {


InputDimension::InputDimension(Dataset& owner, const std::string& name, int id, size_t len) :
    Dimension(owner, name, len), id_(id) {}


InputDimension::~InputDimension() = default;


void InputDimension::clone(Dataset& owner) const {
    owner.add(new OutputDimension(owner, name_, len_));
}


void InputDimension::print(std::ostream& out) const {
    out << "InputDimension[name=" << name_ << "]";
}


int InputDimension::id() const {
    return id_;
}


void InputDimension::realDimensions(std::vector<size_t>& dims) const {
    dims.push_back(len_);
}


}  // namespace mir::netcdf
