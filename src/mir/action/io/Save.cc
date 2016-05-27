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

#include "mir/action/io/Save.h"

#include <iostream>
#include "mir/output/MIROutput.h"
#include "mir/input/MIRInput.h"


namespace mir {
namespace action {

Save::Save(const param::MIRParametrisation &parametrisation, input::MIRInput &input, output::MIROutput &output):
    Action(parametrisation),
    input_(input),
    output_(output) {
}

Save::~Save() {
}

bool Save::sameAs(const Action& other) const {
    const Save* o = dynamic_cast<const Save*>(&other);
    return o && input_.sameAs(o->input_) && output_.sameAs(o->output_);
}

void Save::print(std::ostream &out) const {
    out << "Save[output=" << output_ << "]";
}

void Save::execute(data::MIRField &field) const {
    output_.save(parametrisation_, input_, field);
}

}  // namespace action
}  // namespace mir

