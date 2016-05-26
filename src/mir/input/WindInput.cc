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

#include "mir/input/WindInput.h"

#include <iostream>

#include "mir/data/MIRField.h"
#include "eckit/exception/Exceptions.h"

namespace mir {
namespace input {


WindInput::WindInput(MIRInput &u_component, MIRInput &v_component):
    VectorInput(u_component, v_component) {
}


WindInput::~WindInput() {
}


bool WindInput::sameAs(const MIRInput& other) const {
    const WindInput* o = dynamic_cast<const WindInput*>(&other);
    return o && component1_.sameAs(o->component1_) && component2_.sameAs(o->component2_);
}

void WindInput::print(std::ostream &out) const {
    out << "WindInput[u_component=" << component1_ << ", v_component=" << component2_ << "]";
}

}  // namespace input
}  // namespace mir

