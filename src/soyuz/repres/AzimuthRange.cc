
// File AzimuthRange.cc
// Baudouin Raoult - (c) ECMWF Apr 15

#include "AzimuthRange.h"
#include "soyuz/param/MIRParametrisation.h"
#include "eckit/exception/Exceptions.h"

#include <iostream>

AzimuthRange::AzimuthRange(const MIRParametrisation &parametrisation) {
}

AzimuthRange::AzimuthRange()
{
}


AzimuthRange::~AzimuthRange() {
}


void AzimuthRange::print(std::ostream &out) const {
    out << "AzimuthRange["
        << "]";
}


void AzimuthRange::fill(grib_info &info) const  {
    NOTIMP;
}



static RepresentationBuilder<AzimuthRange> azimuthRange("azimuth_range"); // Name is what is returned by grib_api
