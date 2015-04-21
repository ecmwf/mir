
// File RotatedSH.cc
// Baudouin Raoult - (c) ECMWF Apr 15

#include "soyuz/repres/RotatedSH.h"
#include "soyuz/param/MIRParametrisation.h"
#include "eckit/exception/Exceptions.h"

#include <iostream>

RotatedSH::RotatedSH(const MIRParametrisation &parametrisation) {
}

RotatedSH::RotatedSH() {
}


RotatedSH::~RotatedSH() {
}


void RotatedSH::print(std::ostream &out) const {
    out << "RotatedSH["
        << "]";
}


void RotatedSH::fill(grib_info &info) const  {
    NOTIMP;
}



static RepresentationBuilder<RotatedSH> rotatedSH("rotated_sh"); // Name is what is returned by grib_api
