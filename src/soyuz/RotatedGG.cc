
// File RotatedGG.cc
// Baudouin Raoult - (c) ECMWF Apr 15

#include "RotatedGG.h"
#include "MIRParametrisation.h"
#include "eckit/exception/Exceptions.h"

#include <iostream>

RotatedGG::RotatedGG(const MIRParametrisation &parametrisation) {
}

RotatedGG::RotatedGG()
{
}


RotatedGG::~RotatedGG() {
}


void RotatedGG::print(std::ostream &out) const {
    out << "RotatedGG["
        << "]";
}


void RotatedGG::fill(grib_spec &spec) const  {
    NOTIMP;
}



static RepresentationBuilder<RotatedGG> rotatedGG("rotated_gg"); // Name is what is returned by grib_api
