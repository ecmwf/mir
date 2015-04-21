
// File RotatedLL.cc
// Baudouin Raoult - (c) ECMWF Apr 15

#include "soyuz/repres/RotatedLL.h"
#include "soyuz/param/MIRParametrisation.h"
#include "eckit/exception/Exceptions.h"

#include <iostream>

RotatedLL::RotatedLL(const MIRParametrisation &parametrisation) {
}

RotatedLL::RotatedLL() {
}


RotatedLL::~RotatedLL() {
}


void RotatedLL::print(std::ostream &out) const {
    out << "RotatedLL["
        << "]";
}


void RotatedLL::fill(grib_info &info) const  {
    NOTIMP;
}



static RepresentationBuilder<RotatedLL> rotatedLL("rotated_ll"); // Name is what is returned by grib_api
