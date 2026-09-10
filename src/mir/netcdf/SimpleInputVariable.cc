// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/netcdf/SimpleInputVariable.h"

#include "mir/netcdf/Attribute.h"
#include "mir/netcdf/CellMethodInputVariable.h"
#include "mir/netcdf/CoordinateInputVariable.h"
#include "mir/netcdf/DataInputVariable.h"
#include "mir/netcdf/Dataset.h"
#include "mir/netcdf/Dimension.h"
#include "mir/netcdf/Exceptions.h"
#include "mir/netcdf/SimpleOutputVariable.h"
#include "mir/util/Log.h"


namespace mir::netcdf {

SimpleInputVariable::SimpleInputVariable(Dataset& owner, const std::string& name, int id,
                                         const std::vector<Dimension*>& dimensions) :
    InputVariable(owner, name, id, dimensions) {}

SimpleInputVariable::~SimpleInputVariable() = default;

bool SimpleInputVariable::identified() const {
    // Only SimpleInputVariable are not identified
    return false;
}

Variable* SimpleInputVariable::makeDataVariable() {
    Variable* v = new DataInputVariable(dataset_, name_, id_, dimensions_);
    v->copyAttributes(*this);
    v->setMatrix(matrix_);
    return v;
}

Variable* SimpleInputVariable::makeCoordinateVariable() {
    Variable* v = new CoordinateInputVariable(dataset_, name_, id_, dimensions_);
    v->copyAttributes(*this);
    v->setMatrix(matrix_);
    return v;
}

Variable* SimpleInputVariable::makeCellMethodVariable() {
    Variable* v = new CellMethodInputVariable(dataset_, name_, id_, dimensions_);
    v->copyAttributes(*this);
    v->setMatrix(matrix_);
    return v;
}

Variable* SimpleInputVariable::makeOutputVariable(Dataset& owner, const std::string& name,
                                                  const std::vector<Dimension*>& dimensions) const {
    return new SimpleOutputVariable(owner, name, dimensions);
}

void SimpleInputVariable::print(std::ostream& out) const {
    out << "SimpleInputVariable[name=" << name_ << "]";
}

void SimpleInputVariable::validate() const {
    Log::error() << "Variable '" << name_ << "' is not data, coordinate or cell method." << std::endl;
    // throw exception::MergeError(std::string("Variable ") + name_ + " is not data, coordinate or cell method.");
}

Variable* SimpleInputVariable::addMissingCoordinates() {

    // Check if all dimensions exists as variables

    if (dimensions_.empty()) {
        return this;
    }

    for (auto* d : dimensions_) {
        if (!dataset_.hasVariable(d->name())) {
            return this;
        }
    }

    return makeDataVariable()->addMissingCoordinates();
}

std::vector<std::string> SimpleInputVariable::coordinates() const {

    std::vector<std::string> result;

    auto j = attributes_.find("coordinates");
    if (j != attributes_.end()) {
        std::string s = j->second->asString();
        std::string t;

        for (auto& k : s) {
            if (k == ' ') {
                result.push_back(t);
                t.clear();
            }
            else {
                t.push_back(k);
            }
        }

        if (!t.empty()) {
            result.push_back(t);
        }
    }

    return result;
}

}  // namespace mir::netcdf
