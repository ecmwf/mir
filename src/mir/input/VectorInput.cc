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


#include "mir/input/VectorInput.h"

#include <ostream>

#include "mir/data/MIRField.h"
#include "mir/util/Exceptions.h"


namespace mir {
namespace input {


VectorInput::VectorInput(MIRInput& component1, MIRInput& component2) :
    component1_(component1), component2_(component2) {

    // This is because MIRInput object keep some state
    ASSERT(&component1 != &component2);
}


VectorInput::~VectorInput() = default;


const param::MIRParametrisation& VectorInput::parametrisation(size_t which) const {
    // Assumes that both component (e.g. U and V) have the same parametrisation
    ASSERT(which <= 1);
    return (which == 0) ? component1_.parametrisation() : component2_.parametrisation();
}

grib_handle* VectorInput::gribHandle(size_t which) const {
    // Assumes that both component (e.g. U and V) have the same parametrisation
    ASSERT(which <= 1);
    return (which == 0) ? component1_.gribHandle() : component2_.gribHandle();
}

data::MIRField VectorInput::field() const {
    // Assumes that both component (e.g. U and V) have the same parametrisation
    data::MIRField u = component1_.field();
    data::MIRField v = component2_.field();

    ASSERT(u.dimensions() == 1);
    ASSERT(v.dimensions() == 1);
    ASSERT(u.values(0).size() == v.values(0).size());

    u.update(v.direct(0), 1);

    return u;
}

bool VectorInput::next() {
    bool next_u = component1_.next();
    bool next_v = component2_.next();
    ASSERT(next_u == next_v);
    return next_u;
}


bool VectorInput::sameAs(const MIRInput& other) const {
    const auto* o = dynamic_cast<const VectorInput*>(&other);
    return (o != nullptr) && component1_.sameAs(o->component1_) && component2_.sameAs(o->component2_);
}


void VectorInput::print(std::ostream& out) const {
    out << "VectorInput[" << component1_ << "," << component2_ << "]";
}

size_t VectorInput::dimensions() const {
    return 2;
}

}  // namespace input
}  // namespace mir
