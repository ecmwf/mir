// File NullCropper.cc
// Baudouin Raoult - (c) ECMWF Apr 15

#include "NullCropper.h"

#include <iostream>

NullCropper::NullCropper(const MIRParametrisation& parametrisation):
    Action(parametrisation)
{
}

NullCropper::~NullCropper()
{
}

void NullCropper::print(std::ostream& out) const
{
    out << "NullCropper[]";
}

void NullCropper::execute(MIRField&) const
{
    // Nothing to do
}

static ActionBuilder<NullCropper> nullCropper("crop.noop");
