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
