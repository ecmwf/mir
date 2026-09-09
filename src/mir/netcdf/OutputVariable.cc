// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/netcdf/OutputVariable.h"

#include <netcdf.h>

#include "mir/netcdf/Attribute.h"
#include "mir/netcdf/Codec.h"
#include "mir/netcdf/Dataset.h"
#include "mir/netcdf/Dimension.h"
#include "mir/netcdf/Exceptions.h"
#include "mir/netcdf/Matrix.h"
#include "mir/netcdf/Type.h"


namespace mir::netcdf {


OutputVariable::OutputVariable(Dataset& owner, const std::string& name, const std::vector<Dimension*>& dimensions) :
    Variable(owner, name, dimensions), id_(-1), created_(false) {}


OutputVariable::~OutputVariable() = default;


void OutputVariable::create(int nc) const {

    ASSERT(!created_);
    ASSERT(matrix_ != nullptr);

    int dims[NC_MAX_VAR_DIMS];
    int ndims = 0;
    for (const auto& j : dimensions_) {
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

    for (const auto& j : attributes_) {
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


}  // namespace mir::netcdf
