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

#include "mir/input/VODInput.h"

#include <iostream>

#include "mir/data/MIRField.h"
#include "eckit/exception/Exceptions.h"

namespace mir {
namespace input {


VODInput::VODInput(MIRInput &vorticity, MIRInput &divergence):
    vorticity_(vorticity),
    divergence_(divergence) {
}


VODInput::~VODInput() {
}

void VODInput::print(std::ostream &out) const {
    out << "VODInput[vorticity=" << vorticity_ << ", divergence=" << divergence_ << "]";
}

const param::MIRParametrisation &VODInput::parametrisation() const {
    // Assumes that VO and D are both the same parametrisation
    return vorticity_.parametrisation();
}

data::MIRField *VODInput::field() const {
     // Assumes that VO and D are both the same parametrisation
    data::MIRField *field = vorticity_.field();
    data::MIRField *d = divergence_.field();
    field->values(d->values(), 1);
    delete d;

    return field;
}

}  // namespace input
}  // namespace mir

