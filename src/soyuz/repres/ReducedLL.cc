
// File ReducedLL.cc
// Baudouin Raoult - (c) ECMWF Apr 15

#include "soyuz/repres/ReducedLL.h"
#include "soyuz/param/MIRParametrisation.h"
#include "eckit/exception/Exceptions.h"

#include <iostream>

ReducedLL::ReducedLL(const MIRParametrisation &parametrisation) {
}

ReducedLL::ReducedLL()
{
}


ReducedLL::~ReducedLL() {
}


void ReducedLL::print(std::ostream &out) const {
    out << "ReducedLL["
        << "]";
}


void ReducedLL::fill(grib_info &info) const  {
    NOTIMP;
}



static RepresentationBuilder<ReducedLL> reducedLL("reduced_ll"); // Name is what is returned by grib_api
