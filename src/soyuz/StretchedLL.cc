
// File StretchedLL.cc
// Baudouin Raoult - (c) ECMWF Apr 15

#include "StretchedLL.h"
#include "MIRParametrisation.h"
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


void StretchedLL::fill(grib_spec &spec) const  {
    NOTIMP;
}



static RepresentationBuilder<StretchedLL> stretchedLL("stretched_ll"); // Name is what is returned by grib_api
