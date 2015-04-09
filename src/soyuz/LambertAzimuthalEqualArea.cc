
// File LambertAzimuthalEqualArea.cc
// Baudouin Raoult - (c) ECMWF Apr 15

#include "LambertAzimuthalEqualArea.h"
#include "MIRParametrisation.h"
#include "eckit/exception/Exceptions.h"

#include <iostream>

LambertAzimuthalEqualArea::LambertAzimuthalEqualArea(const MIRParametrisation &parametrisation) {
}

LambertAzimuthalEqualArea::LambertAzimuthalEqualArea()
{
}


LambertAzimuthalEqualArea::~LambertAzimuthalEqualArea() {
}


void LambertAzimuthalEqualArea::print(std::ostream &out) const {
    out << "LambertAzimuthalEqualArea["
        << "]";
}


void LambertAzimuthalEqualArea::fill(grib_spec &spec) const  {
    NOTIMP;
}



static RepresentationBuilder<LambertAzimuthalEqualArea> lambertAzimuthalEqualArea("lambert_azimuthal_equal_area"); // Name is what is returned by grib_api
