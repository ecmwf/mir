/*
 * (C) Copyright 1996-2013 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

// Baudouin Raoult - ECMWF Jan 2015

#include "mir/netcdf/DummyMatrix.h"

#include "mir/netcdf/Type.h"
#include "mir/netcdf/Variable.h"

#include <ostream>

namespace mir {
namespace netcdf {

DummyMatrix::DummyMatrix(const Variable &v):
    Matrix(v.matrix()->type(), v.name(), v.matrix()->size()),
    other_(v.matrix())
{
    other_->attach();
}

DummyMatrix::~DummyMatrix()
{
    other_->detach();
}

void DummyMatrix::print(std::ostream &out) const {
    out << "DummyMatrix[type=" << *type_ << ",name=" << name_ << ",other=" << *other_ << "]";
}

}
}
