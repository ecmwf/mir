// File Gridded2GriddedInterpolation.cc
// Baudouin Raoult - (c) ECMWF Apr 15

#include "soyuz/action/Gridded2GriddedInterpolation.h"

#include "eckit/exception/Exceptions.h"
#include "soyuz/method/Method.h"
#include "soyuz/param/MIRParametrisation.h"

#include <iostream>
#include <memory>

Gridded2GriddedInterpolation::Gridded2GriddedInterpolation(const MIRParametrisation& parametrisation):
    Action(parametrisation) {
}

Gridded2GriddedInterpolation::~Gridded2GriddedInterpolation() {
}

void Gridded2GriddedInterpolation::print(std::ostream& out) const {
    out << "Gridded2GriddedInterpolation[]";
}

void Gridded2GriddedInterpolation::execute(MIRField& field) const {
    NOTIMP;

    std::string name = "bilinear";

    // Here we need some ugnly logic again
    if(0) {
        std::string param;
        ASSERT(parametrisation_.get("param", param));
        if(param == "large_scale_precipitiaon") { // Thisshould be a lookup in a config file somewhere
            name = "mass_conserving";
        }
    }

    std::auto_ptr<Method> method(MethodFactory::build(name, parametrisation_));
    method->execute(field);

    // TODO: Use Representation and MIRfield to create Atlas structures
    // TODO: Connect "Methods" and "mir/Weigths"


}

static ActionBuilder<Gridded2GriddedInterpolation> grid2grid("interpolate.grid2grid");
