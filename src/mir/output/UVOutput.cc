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

#include "mir/output/UVOutput.h"

#include <iostream>

#include "eckit/exception/Exceptions.h"


namespace mir {
namespace output {


UVOutput::UVOutput(MIROutput &u_component, MIROutput &v_component):
    u_component_(u_component),
    v_component_(v_component) {
}


UVOutput::~UVOutput() {
}

void UVOutput::print(std::ostream &out) const {
    out << "UVOutput[u_component=" << u_component_ << ", v_component=" << v_component_ << "]";
}

void UVOutput::copy(mir::param::MIRParametrisation const &, mir::input::MIRInput &) {
    NOTIMP;
}

void UVOutput::save(mir::param::MIRParametrisation const &, mir::input::MIRInput &, mir::data::MIRField &) {
    NOTIMP;
}

}  // namespace output
}  // namespace mir

