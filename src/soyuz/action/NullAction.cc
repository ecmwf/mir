// File NullAction.cc
// Baudouin Raoult - (c) ECMWF Apr 15

#include "soyuz/action/NullAction.h"

#include <iostream>

NullAction::NullAction(const MIRParametrisation& parametrisation):
    Action(parametrisation) {
}

NullAction::~NullAction() {
}

void NullAction::print(std::ostream& out) const {
    out << "NullAction[]";
}

void NullAction::execute(MIRField&) const {
    // Nothing to do
}

static ActionBuilder<NullAction> nullCropper("action.noop");
