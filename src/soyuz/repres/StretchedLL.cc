
// File StretchedLL.cc
// Baudouin Raoult - (c) ECMWF Apr 15

#include "soyuz/repres/StretchedLL.h"
#include "soyuz/param/MIRParametrisation.h"
#include "eckit/exception/Exceptions.h"

#include <iostream>

StretchedLL::StretchedLL(const MIRParametrisation &parametrisation) {
}

StretchedLL::StretchedLL()
{
}


StretchedLL::~StretchedLL() {
}


void StretchedLL::print(std::ostream &out) const {
    out << "StretchedLL["
        << "]";
}


void StretchedLL::fill(grib_info &info) const  {
    NOTIMP;
}



static RepresentationBuilder<StretchedLL> stretchedLL("stretched_ll"); // Name is what is returned by grib_api
