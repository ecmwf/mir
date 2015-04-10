
// File StretchedGG.cc
// Baudouin Raoult - (c) ECMWF Apr 15

#include "soyuz/repres/StretchedGG.h"
#include "soyuz/param/MIRParametrisation.h"
#include "eckit/exception/Exceptions.h"

#include <iostream>

StretchedGG::StretchedGG(const MIRParametrisation &parametrisation) {
}

StretchedGG::StretchedGG()
{
}


StretchedGG::~StretchedGG() {
}


void StretchedGG::print(std::ostream &out) const {
    out << "StretchedGG["
        << "]";
}


void StretchedGG::fill(grib_info &info) const  {
    NOTIMP;
}



static RepresentationBuilder<StretchedGG> stretchedGG("stretched_gg"); // Name is what is returned by grib_api
