
// File TransverseMercator.cc
// Baudouin Raoult - (c) ECMWF Apr 15

#include "TransverseMercator.h"
#include "soyuz/param/MIRParametrisation.h"
#include "eckit/exception/Exceptions.h"

#include <iostream>

TransverseMercator::TransverseMercator(const MIRParametrisation &parametrisation) {
}

TransverseMercator::TransverseMercator()
{
}


TransverseMercator::~TransverseMercator() {
}


void TransverseMercator::print(std::ostream &out) const {
    out << "TransverseMercator["
        << "]";
}


void TransverseMercator::fill(grib_info &info) const  {
    NOTIMP;
}



static RepresentationBuilder<TransverseMercator> transverseMercator("transverse_mercator"); // Name is what is returned by grib_api
