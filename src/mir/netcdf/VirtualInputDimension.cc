// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/netcdf/VirtualInputDimension.h"

#include <ostream>

#include "mir/netcdf/Dataset.h"
#include "mir/netcdf/VirtualOutputDimension.h"


namespace mir::netcdf {


VirtualInputDimension::VirtualInputDimension(Dataset& owner, const std::string& name) : Dimension(owner, name, 1) {}


VirtualInputDimension::~VirtualInputDimension() = default;


void VirtualInputDimension::clone(Dataset& owner) const {
    owner.add(new VirtualOutputDimension(owner, name_));
}


void VirtualInputDimension::print(std::ostream& out) const {
    out << "VirtualInputDimension[name=" << name_ << "]";
}


int VirtualInputDimension::id() const {
    return -1;
}


void VirtualInputDimension::realDimensions(std::vector<size_t>& /*dims*/) const {
    // empty
}


}  // namespace mir::netcdf
