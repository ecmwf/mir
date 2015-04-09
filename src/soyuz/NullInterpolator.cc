// File NullInterpolator.cc
// Baudouin Raoult - (c) ECMWF Apr 15

#include "NullInterpolator.h"

#include <iostream>

NullInterpolator::NullInterpolator(const MIRParametrisation& parametrisation):
    Action(parametrisation)
{
}

NullInterpolator::~NullInterpolator()
{
}


void NullInterpolator::print(std::ostream& out) const
{
    out << "NullInterpolator[]";
}

void NullInterpolator::execute(MIRField&) const
{
    // Nothing to do
}

static ActionBuilder<NullInterpolator> nullInterpolator("interpolate.noop");
