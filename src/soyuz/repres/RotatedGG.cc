
// File RotatedGG.cc
// Baudouin Raoult - (c) ECMWF Apr 15

#include "soyuz/repres/RotatedGG.h"
#include "soyuz/param/MIRParametrisation.h"
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


void RotatedGG::fill(grib_info &info) const  {
    NOTIMP;
}



static RepresentationBuilder<RotatedGG> rotatedGG("rotated_gg"); // Name is what is returned by grib_api
