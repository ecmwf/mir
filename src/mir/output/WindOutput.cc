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

#include "mir/output/WindOutput.h"

#include <iostream>

#include "eckit/exception/Exceptions.h"
#include "mir/data/MIRField.h"
#include "mir/param/RuntimeParametrisation.h"
#include "mir/repres/Representation.h"


namespace mir {
namespace output {


WindOutput::WindOutput(MIROutput &u_component, MIROutput &v_component):
    VectorOutput(u_component, v_component) {
}


WindOutput::~WindOutput() {
}


bool WindOutput::sameAs(const MIROutput& other) const {
    const WindOutput* o = dynamic_cast<const WindOutput*>(&other);
    return o && component1_.sameAs(o->component1_) && component2_.sameAs(o->component2_);
}

void WindOutput::print(std::ostream &out) const {
    out << "WindOutput[u_component=" << component1_ << ", v_component=" << component2_ << "]";
}


}  // namespace output
}  // namespace mir

