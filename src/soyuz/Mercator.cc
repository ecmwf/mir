
// File Mercator.cc
// Baudouin Raoult - (c) ECMWF Apr 15

#include "Mercator.h"
#include "MIRParametrisation.h"
#include "eckit/exception/Exceptions.h"

#include <iostream>

Mercator::Mercator(const MIRParametrisation &parametrisation) {
}

Mercator::Mercator()
{
}


Mercator::~Mercator() {
}


void Mercator::print(std::ostream &out) const {
    out << "Mercator["
        << "]";
}


void Mercator::fill(grib_spec &spec) const  {
    NOTIMP;
}



static RepresentationBuilder<Mercator> mercator("mercator"); // Name is what is returned by grib_api
