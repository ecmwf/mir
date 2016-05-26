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
#include "mir/data/MIRField.h"
#include "mir/param/RuntimeParametrisation.h"
#include "mir/repres/Representation.h"


namespace mir {
namespace output {


UVOutput::UVOutput(MIROutput &u_component, MIROutput &v_component):
    VectorOutput(u_component, v_component) {
}


UVOutput::~UVOutput() {
}

bool UVOutput::sameAs(const MIROutput& other) const {
    const UVOutput* o = dynamic_cast<const UVOutput*>(&other);
    return o && component1_.sameAs(o->component1_) && component2_.sameAs(o->component2_);
}

void UVOutput::print(std::ostream &out) const {
    out << "UVOutput[u_component=" << component1_ << ", v_component=" << component2_ << "]";
}

long UVOutput::component1ParamId(input::MIRInput &) const  {
    return 131; // TODO: Find a better way
}

long UVOutput::component2ParamId(input::MIRInput &) const {
    return 132; // TODO: Find a better way
}


}  // namespace output
}  // namespace mir

