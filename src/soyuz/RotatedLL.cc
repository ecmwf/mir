
// File RotatedLL.cc
// Baudouin Raoult - (c) ECMWF Apr 15

#include "RotatedLL.h"
#include "MIRParametrisation.h"
#include "eckit/exception/Exceptions.h"

#include <iostream>

RotatedLL::RotatedLL(const MIRParametrisation &parametrisation) {
}

RotatedLL::RotatedLL()
{
}


RotatedLL::~RotatedLL() {
}


void RotatedLL::print(std::ostream &out) const {
    out << "RotatedLL["
        << "]";
}


void RotatedLL::fill(grib_spec &spec) const  {
    NOTIMP;
}



static RepresentationBuilder<RotatedLL> rotatedLL("rotated_ll"); // Name is what is returned by grib_api
