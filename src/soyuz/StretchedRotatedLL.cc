
// File StretchedRotatedLL.cc
// Baudouin Raoult - (c) ECMWF Apr 15

#include "StretchedRotatedLL.h"
#include "MIRParametrisation.h"
#include "eckit/exception/Exceptions.h"

#include <iostream>

StretchedRotatedLL::StretchedRotatedLL(const MIRParametrisation &parametrisation) {
}

StretchedRotatedLL::StretchedRotatedLL()
{
}


StretchedRotatedLL::~StretchedRotatedLL() {
}


void StretchedRotatedLL::print(std::ostream &out) const {
    out << "StretchedRotatedLL["
        << "]";
}


void StretchedRotatedLL::fill(grib_spec &spec) const  {
    NOTIMP;
}



static RepresentationBuilder<StretchedRotatedLL> stretchedRotatedLL("stretched_rotated_ll"); // Name is what is returned by grib_api
