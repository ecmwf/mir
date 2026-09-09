// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/netcdf/DummyMatrix.h"

#include "mir/netcdf/Type.h"
#include "mir/netcdf/Variable.h"

#include <ostream>

namespace mir::netcdf {

DummyMatrix::DummyMatrix(const Variable& v) :
    Matrix(v.matrix()->type(), v.name(), v.matrix()->size()), other_(v.matrix()) {
    other_->attach();
}

DummyMatrix::~DummyMatrix() {
    other_->detach();
}

void DummyMatrix::print(std::ostream& out) const {
    out << "DummyMatrix[type=" << *type_ << ",name=" << name_ << ",other=" << *other_ << "]";
}

}  // namespace mir::netcdf
