// File NullTransformer.cc
// Baudouin Raoult - (c) ECMWF Apr 15

#include "NullTransformer.h"

#include <iostream>

NullTransformer::NullTransformer(const MIRParametrisation& parametrisation):
    Action(parametrisation)
{
}

NullTransformer::~NullTransformer()
{
}

void NullTransformer::print(std::ostream& out) const
{
    out << "NullTransformer[]";
}

void NullTransformer::execute(MIRField&) const
{
    // Nothing to do
}

static ActionBuilder<NullTransformer> nullTransformer("transform.noop");
