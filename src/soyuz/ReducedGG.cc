
// File ReducedGG.cc
// Baudouin Raoult - (c) ECMWF Apr 15

#include "ReducedGG.h"
#include "MIRParametrisation.h"
#include "eckit/exception/Exceptions.h"

#include <iostream>

ReducedGG::ReducedGG(const MIRParametrisation &parametrisation) {
}

ReducedGG::ReducedGG()
{
}


ReducedGG::~ReducedGG() {
}


void ReducedGG::print(std::ostream &out) const {
    out << "ReducedGG["
        << "]";
}


void ReducedGG::fill(grib_info &info) const  {
    NOTIMP;
}



static RepresentationBuilder<ReducedGG> reducedGG("reduced_gg"); // Name is what is returned by grib_api
