
// File RegularGG.cc
// Baudouin Raoult - (c) ECMWF Apr 15

#include "RegularGG.h"
#include "soyuz/param/MIRParametrisation.h"
#include "eckit/exception/Exceptions.h"

#include <iostream>

RegularGG::RegularGG(const MIRParametrisation &parametrisation) {
}

RegularGG::RegularGG()
{
}


RegularGG::~RegularGG() {
}


void RegularGG::print(std::ostream &out) const {
    out << "RegularGG["
        << "]";
}


void RegularGG::fill(grib_info &info) const  {
    NOTIMP;
}



static RepresentationBuilder<RegularGG> regularGG("regular_gg"); // Name is what is returned by grib_api
