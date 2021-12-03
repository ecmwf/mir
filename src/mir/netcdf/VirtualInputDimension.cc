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


#include "mir/netcdf/VirtualInputDimension.h"

#include <ostream>

#include "mir/netcdf/Dataset.h"
#include "mir/netcdf/VirtualOutputDimension.h"


namespace mir {
namespace netcdf {


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


}  // namespace netcdf
}  // namespace mir
