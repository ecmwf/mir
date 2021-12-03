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


#include "mir/netcdf/OutputVariable.h"

#include <netcdf.h>

#include "mir/netcdf/Attribute.h"
#include "mir/netcdf/Codec.h"
#include "mir/netcdf/Dataset.h"
#include "mir/netcdf/Dimension.h"
#include "mir/netcdf/Exceptions.h"
#include "mir/netcdf/Matrix.h"
#include "mir/netcdf/Type.h"


namespace mir {
namespace netcdf {


OutputVariable::OutputVariable(Dataset& owner, const std::string& name, const std::vector<Dimension*>& dimensions) :
    Variable(owner, name, dimensions), id_(-1), created_(false) {}


OutputVariable::~OutputVariable() = default;


void OutputVariable::create(int nc) const {

    ASSERT(!created_);
    ASSERT(matrix_ != nullptr);

    int dims[NC_MAX_VAR_DIMS];
    int ndims = 0;
    for (auto& j : dimensions_) {
        if (j->inUse()) {
            dims[ndims++] = j->id();
        }
    }

    std::string name = ncname();

    NC_CALL(nc_def_var(nc, name.c_str(), matrix_->type().code(), ndims, dims, &id_), dataset_.path());

    Codec* codec = matrix_->codec();
    if (codec != nullptr) {
        Variable* self = const_cast<OutputVariable*>(this);
        codec->addAttributes(*self);
    }

    created_ = true;

    for (auto& j : attributes_) {
        (j.second)->create(nc);
    }
}


void OutputVariable::save(int nc) const {
    ASSERT(created_);
    matrix_->save(nc, id_, path());

    Codec* codec = matrix_->codec();
    if (codec != nullptr) {
        codec->updateAttributes(nc, id_, path());
    }
}


void OutputVariable::print(std::ostream& out) const {
    out << "OutputVariable[name=" << name_ << "]";
}


int OutputVariable::varid() const {
    ASSERT(created_);
    ASSERT(id_ >= 0);
    return id_;
}


}  // namespace netcdf
}  // namespace mir
