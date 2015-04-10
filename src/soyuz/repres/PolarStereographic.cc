
// File PolarStereographic.cc
// Baudouin Raoult - (c) ECMWF Apr 15

#include "PolarStereographic.h"
#include "soyuz/param/MIRParametrisation.h"
#include "eckit/exception/Exceptions.h"

#include <iostream>

PolarStereographic::PolarStereographic(const MIRParametrisation &parametrisation) {
}

PolarStereographic::PolarStereographic()
{
}


PolarStereographic::~PolarStereographic() {
}


void PolarStereographic::print(std::ostream &out) const {
    out << "PolarStereographic["
        << "]";
}


void PolarStereographic::fill(grib_info &info) const  {
    NOTIMP;
}



static RepresentationBuilder<PolarStereographic> polarStereographic("polar_stereographic"); // Name is what is returned by grib_api
