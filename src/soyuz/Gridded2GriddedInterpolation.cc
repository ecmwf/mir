// File Gridded2GriddedInterpolation.cc
// Baudouin Raoult - (c) ECMWF Apr 15

#include "Gridded2GriddedInterpolation.h"

#include "eckit/exception/Exceptions.h"
#include "Method.h"

#include <iostream>
#include <memory>

Gridded2GriddedInterpolation::Gridded2GriddedInterpolation(const MIRParametrisation& parametrisation):
    Action(parametrisation)
{
}

Gridded2GriddedInterpolation::~Gridded2GriddedInterpolation()
{
}

void Gridded2GriddedInterpolation::print(std::ostream& out) const
{
    out << "Gridded2GriddedInterpolation[]";
}

void Gridded2GriddedInterpolation::execute(MIRField&) const
{
    NOTIMP;

    std::auto_ptr<Method> method(MethodFactory::build(parametrisation_));

    // TODO: Use Representation and MIRfield to create Atlas structures
    // TODO: Connect "Methods" and "mir/Weigths"


}

static ActionBuilder<Gridded2GriddedInterpolation> grid2grid("interpolate.grid2grid");
