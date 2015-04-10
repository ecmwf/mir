
// File StretchedRotatedSH.cc
// Baudouin Raoult - (c) ECMWF Apr 15

#include "StretchedRotatedSH.h"
#include "MIRParametrisation.h"
#include "eckit/exception/Exceptions.h"

#include <iostream>

StretchedRotatedSH::StretchedRotatedSH(const MIRParametrisation &parametrisation) {
}

StretchedRotatedSH::StretchedRotatedSH()
{
}


StretchedRotatedSH::~StretchedRotatedSH() {
}


void StretchedRotatedSH::print(std::ostream &out) const {
    out << "StretchedRotatedSH["
        << "]";
}


void StretchedRotatedSH::fill(grib_info &info) const  {
    NOTIMP;
}



static RepresentationBuilder<StretchedRotatedSH> stretchedRotatedSH("stretched_rotated_sh"); // Name is what is returned by grib_api
