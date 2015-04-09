
// File EquatorialAzimuthalEquidistant.cc
// Baudouin Raoult - (c) ECMWF Apr 15

#include "EquatorialAzimuthalEquidistant.h"
#include "MIRParametrisation.h"
#include "eckit/exception/Exceptions.h"

#include <iostream>

EquatorialAzimuthalEquidistant::EquatorialAzimuthalEquidistant(const MIRParametrisation &parametrisation) {
}

EquatorialAzimuthalEquidistant::EquatorialAzimuthalEquidistant()
{
}


EquatorialAzimuthalEquidistant::~EquatorialAzimuthalEquidistant() {
}


void EquatorialAzimuthalEquidistant::print(std::ostream &out) const {
    out << "EquatorialAzimuthalEquidistant["
        << "]";
}


void EquatorialAzimuthalEquidistant::fill(grib_spec &spec) const  {
    NOTIMP;
}



static RepresentationBuilder<EquatorialAzimuthalEquidistant> equatorialAzimuthalEquidistant("equatorial_azimuthal_equidistant"); // Name is what is returned by grib_api
