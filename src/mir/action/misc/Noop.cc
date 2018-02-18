/*
 * (C) Copyright 1996- ECMWF.
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

#include "mir/action/misc/Noop.h"

#include <iostream>

// #include "eckit/memory/ScopedPtr.h"

#include "mir/action/context/Context.h"
#include "mir/param/MIRParametrisation.h"
// #include "mir/repres/Iterator.h"
// #include "mir/repres/Representation.h"
#include "mir/data/MIRField.h"


namespace mir {
namespace action {


Noop::Noop(const param::MIRParametrisation &parametrisation):
    Action(parametrisation) {

}


Noop::~Noop() {
}


bool Noop::sameAs(const Action& other) const {
    const Noop* o = dynamic_cast<const Noop*>(&other);
    return o;
}

void Noop::print(std::ostream &out) const {
    out << "Noop[]";
}


void Noop::execute(context::Context & ctx) const {
}

const char* Noop::name() const {
    return "Noop";
}


namespace {
static ActionBuilder< Noop > action("noop");
}


}  // namespace action
}  // namespace mir

