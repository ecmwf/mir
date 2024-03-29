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


#include "mir/netcdf/VirtualOutputDimension.h"

#include <netcdf.h>

#include "mir/netcdf/Dataset.h"
#include "mir/netcdf/Exceptions.h"


namespace mir::netcdf {


VirtualOutputDimension::VirtualOutputDimension(Dataset& owner, const std::string& name) :
    Dimension(owner, name, 1), id_(-1), created_(false), grown_(false) {}


VirtualOutputDimension::~VirtualOutputDimension() = default;


void VirtualOutputDimension::create(int nc) const {
    ASSERT(len_ > 1);
    ASSERT(!created_);

    NC_CALL(nc_def_dim(nc, name_.c_str(), len_, &id_), owner_.path());

    created_ = true;
}


bool VirtualOutputDimension::inUse() const {
    return len_ > 1;
}


void VirtualOutputDimension::print(std::ostream& out) const {
    out << "VirtualOutputDimension[name=" << name_ << ",size=" << len_ << "]";
}


int VirtualOutputDimension::id() const {
    ASSERT(created_);
    return id_;
}


void VirtualOutputDimension::grow(size_t count) {
    ASSERT(!created_);
    ASSERT(count >= len_);
    len_   = count;
    grown_ = true;
}


}  // namespace mir::netcdf
