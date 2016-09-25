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

#include "mir/action/misc/SetParameter.h"

#include <iostream>

// #include "eckit/memory/ScopedPtr.h"

#include "mir/action/context/Context.h"
#include "mir/param/MIRParametrisation.h"
// #include "mir/repres/Iterator.h"
// #include "mir/repres/Representation.h"
#include "mir/data/MIRField.h"


namespace mir {
namespace action {


SetParameter::SetParameter(const param::MIRParametrisation &parametrisation):
    Action(parametrisation) {

    ASSERT(parametrisation_.get("parameter", parameter_));
}


SetParameter::~SetParameter() {
}


bool SetParameter::sameAs(const Action& other) const {
    const SetParameter* o = dynamic_cast<const SetParameter*>(&other);
    return o && (parameter_ == o->parameter_);
}

void SetParameter::print(std::ostream &out) const {
    out << "SetParameter[" << parameter_ << "]";
}


void SetParameter::execute(context::Context & ctx) const {
    data::MIRField& field = ctx.field();
    field.metadata(0, "paramId", parameter_);
}


namespace {
static ActionBuilder< SetParameter > action("set.parameter");
}


}  // namespace action
}  // namespace mir

