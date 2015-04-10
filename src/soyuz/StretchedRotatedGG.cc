
// File StretchedRotatedGG.cc
// Baudouin Raoult - (c) ECMWF Apr 15

#include "StretchedRotatedGG.h"
#include "MIRParametrisation.h"
#include "eckit/exception/Exceptions.h"

#include <iostream>

StretchedRotatedGG::StretchedRotatedGG(const MIRParametrisation &parametrisation) {
}

StretchedRotatedGG::StretchedRotatedGG()
{
}


StretchedRotatedGG::~StretchedRotatedGG() {
}


void StretchedRotatedGG::print(std::ostream &out) const {
    out << "StretchedRotatedGG["
        << "]";
}


void StretchedRotatedGG::fill(grib_info &info) const  {
    NOTIMP;
}



static RepresentationBuilder<StretchedRotatedGG> stretchedRotatedGG("stretched_rotated_gg"); // Name is what is returned by grib_api
