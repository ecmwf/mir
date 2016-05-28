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

#include "mir/output/ForwardOutput.h"

#include <iostream>

#include "eckit/exception/Exceptions.h"

#include "eckit/io/Length.h"


namespace mir {
namespace output {


ForwardOutput::ForwardOutput(MIROutput &output):
    output_(output) {
}

ForwardOutput::~ForwardOutput() {
}

void ForwardOutput::copy(const param::MIRParametrisation &param, input::MIRInput &input) {
    output_.copy(param, input);
}

void ForwardOutput::save(const param::MIRParametrisation &param, input::MIRInput &input, data::MIRField &field) {
    output_.save(param, input, field);
}

eckit::Length ForwardOutput::total() const {
    return output_.total();
}

bool ForwardOutput::sameAs(const MIROutput &other) const {
    // TODO: For now....
    return false;
}

void ForwardOutput::print(std::ostream &out) const {
    out << "ForwardOutput[" << output_ << "]";
}

}  // namespace output
}  // namespace mir

