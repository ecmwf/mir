// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/netcdf/MergeCoordinateMatrix.h"

#include <ostream>

#include "mir/netcdf/Mapper.h"
#include "mir/netcdf/Type.h"


namespace mir::netcdf {


MergeCoordinateMatrix::MergeCoordinateMatrix(Matrix* out, Matrix* in, size_t size) :
    Matrix(Type::lookup(out->type(), in->type()), out->name(), size), out_(out), in_(in) {
    out_->attach();
    in_->attach();
}


MergeCoordinateMatrix::~MergeCoordinateMatrix() {
    out_->detach();
    in_->detach();
}


void MergeCoordinateMatrix::print(std::ostream& out) const {
    out << "MergeCoordinateMatrix[name=" << name_ << ", ptr=" << this << ", size=" << size_ << "]";
}


void MergeCoordinateMatrix::dumpTree(std::ostream& out, size_t depth) const {
    out << std::string(depth, ' ') << *this << std::endl;
    out_->dumpTree(out, depth + 1);
    in_->dumpTree(out, depth + 1);
}


}  // namespace mir::netcdf
