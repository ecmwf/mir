// File Sh2GriddedTransform.cc
// Baudouin Raoult - (c) ECMWF Apr 15

#include "Sh2GriddedTransform.h"

#include "eckit/exception/Exceptions.h"

#include <iostream>

Sh2GriddedTransform::Sh2GriddedTransform(const MIRParametrisation& parametrisation):
    Action(parametrisation)
{
}

Sh2GriddedTransform::~Sh2GriddedTransform()
{
}

void Sh2GriddedTransform::print(std::ostream& out) const
{
    out << "Sh2GriddedTransform[]";
}

void Sh2GriddedTransform::execute(MIRField&) const
{
    NOTIMP;
}

static ActionBuilder<Sh2GriddedTransform> nullCropper("action.noop");
