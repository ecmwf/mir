// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/action/misc/Noop.h"

#include <ostream>


namespace mir::action {


bool Noop::sameAs(const Action& other) const {
    const auto* o = dynamic_cast<const Noop*>(&other);
    return (o != nullptr);
}


void Noop::print(std::ostream& out) const {
    out << "Noop[]";
}


void Noop::execute(context::Context& /*unused*/) const {}


const char* Noop::name() const {
    return "Noop";
}


static const ActionBuilder<Noop> __action("noop");


}  // namespace mir::action
