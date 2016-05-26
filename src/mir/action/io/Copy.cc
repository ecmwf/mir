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

#include "mir/action/io/Copy.h"

#include <iostream>
#include "mir/output/MIROutput.h"
#include "mir/input/MIRInput.h"


namespace mir {
namespace action {

Copy::Copy(const param::MIRParametrisation &parametrisation, input::MIRInput &input, output::MIROutput &output):
    Action(parametrisation),
    input_(input),
    output_(output) {
}

Copy::~Copy() {
}

bool Copy::sameAs(const Action& other) const {
    const Copy* o = dynamic_cast<const Copy*>(&other);
    return o && input_.sameAs(o->input_) && output_.sameAs(o->output_);
}

void Copy::print(std::ostream &out) const {
    out << "Copy[output=" << output_ << "]";
}

void Copy::execute(data::MIRField &field) const {
    output_.copy(parametrisation_, input_);
}

bool Copy::needField() const {
    return false;
}

}  // namespace action
}  // namespace mir

