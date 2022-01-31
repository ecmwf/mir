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


#include "mir/netcdf/InputVariable.h"

#include "mir/netcdf/Attribute.h"
#include "mir/netcdf/Dataset.h"
#include "mir/netcdf/Dimension.h"
#include "mir/netcdf/Exceptions.h"


namespace mir {
namespace netcdf {


InputVariable::InputVariable(Dataset& owner, const std::string& name, int id,
                             const std::vector<Dimension*>& dimensions) :
    Variable(owner, name, dimensions), id_(id) {}


InputVariable::~InputVariable() = default;


int InputVariable::varid() const {
    ASSERT(id_ >= 0);
    return id_;
}


Variable* InputVariable::clone(Dataset& owner) const {

    std::vector<Dimension*> dimensions;
    dimensions.reserve(dimensions_.size());

    for (const auto& j : dimensions_) {
        dimensions.push_back(owner.findDimension(j->name()));
    }

    Variable* v = makeOutputVariable(owner, name_, dimensions);
    v->setMatrix(matrix_);

    for (const auto& j : attributes_) {
        (j.second)->clone(*v);
    }

    owner.add(v);

    return v;
}


void InputVariable::print(std::ostream& out) const {
    out << "InputVariable[name=" << name_ << "]";
}


}  // namespace netcdf
}  // namespace mir
