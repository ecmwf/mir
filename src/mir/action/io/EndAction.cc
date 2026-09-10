// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/action/io/EndAction.h"

#include <ostream>

#include "mir/output/MIROutput.h"


namespace mir::action::io {


EndAction::EndAction(const param::MIRParametrisation& param, output::MIROutput& output) :
    Action(param), output_(output) {}


bool EndAction::sameAs(const Action& other) const {
    const auto* o = dynamic_cast<const EndAction*>(&other);
    return (o != nullptr) && output_.sameAs(o->output_) &&
           o->output_.sameParametrisation(parametrisation(), o->parametrisation());
}


bool EndAction::isEndAction() const {
    return true;
}


void EndAction::print(std::ostream& out) const {
    out << name() << "[";
    if (output().printParametrisation(out, parametrisation())) {
        out << ",";
    }
    out << "output=" << output() << "]";
}


void EndAction::custom(std::ostream& out) const {
    out << name() << "[";
    if (output().printParametrisation(out, parametrisation())) {
        out << ",";
    }
    out << "output=...]";
}


}  // namespace mir::action::io
