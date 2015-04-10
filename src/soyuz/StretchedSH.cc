
// File StretchedSH.cc
// Baudouin Raoult - (c) ECMWF Apr 15

#include "StretchedSH.h"
#include "MIRParametrisation.h"
#include "eckit/exception/Exceptions.h"

#include <iostream>

StretchedSH::StretchedSH(const MIRParametrisation &parametrisation) {
}

StretchedSH::StretchedSH()
{
}


StretchedSH::~StretchedSH() {
}


void StretchedSH::print(std::ostream &out) const {
    out << "StretchedSH["
        << "]";
}


void StretchedSH::fill(grib_info &info) const  {
    NOTIMP;
}



static RepresentationBuilder<StretchedSH> stretchedSH("stretched_sh"); // Name is what is returned by grib_api
