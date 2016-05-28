/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date Apr 2015

#include "mir/input/VectorInput.h"

#include <iostream>

#include "mir/data/MIRField.h"
#include "eckit/exception/Exceptions.h"

namespace mir {
namespace input {


VectorInput::VectorInput(MIRInput &component1, MIRInput &component2):
    component1_(component1),
    component2_(component2) {

    // This is because MITInput object keep some state
    ASSERT(&component1 != &component2);
}


VectorInput::~VectorInput() {
}


const param::MIRParametrisation &VectorInput::parametrisation() const {
    // Assumes that both component (e.g. U and V) have the same parametrisation
    return component1_.parametrisation();
}

grib_handle* VectorInput::gribHandle() const {
    // Assumes that both component (e.g. U and V) have the same parametrisation
    return component1_.gribHandle();
}

data::MIRField *VectorInput::field() const {
    // Assumes that both component (e.g. U and V) have the same parametrisation
    data::MIRField *u = component1_.field();
    data::MIRField *v = component2_.field();

    ASSERT(u->dimensions() == 1);
    ASSERT(v->dimensions() == 1);

    u->update(v->direct(0), 1);
    delete v;

    return u;
}

bool VectorInput::next() {
    bool next_u = component1_.next();
    bool next_v = component2_.next();
    ASSERT(next_u == next_v);
    return next_u;
}



}  // namespace input
}  // namespace mir

