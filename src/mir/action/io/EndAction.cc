/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


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
