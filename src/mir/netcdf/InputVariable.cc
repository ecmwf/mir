// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/netcdf/InputVariable.h"

#include "mir/netcdf/Attribute.h"
#include "mir/netcdf/Dataset.h"
#include "mir/netcdf/Dimension.h"
#include "mir/netcdf/Exceptions.h"


namespace mir::netcdf {


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


}  // namespace mir::netcdf
