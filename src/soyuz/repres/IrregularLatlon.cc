
// File IrregularLatlon.cc
// Baudouin Raoult - (c) ECMWF Apr 15

#include "soyuz/repres/IrregularLatlon.h"
#include "soyuz/param/MIRParametrisation.h"
#include "eckit/exception/Exceptions.h"

#include <iostream>

IrregularLatlon::IrregularLatlon(const MIRParametrisation &parametrisation) {
}

IrregularLatlon::IrregularLatlon() {
}


IrregularLatlon::~IrregularLatlon() {
}


void IrregularLatlon::print(std::ostream &out) const {
    out << "IrregularLatlon["
        << "]";
}


void IrregularLatlon::fill(grib_info &info) const  {
    NOTIMP;
}



static RepresentationBuilder<IrregularLatlon> irregularLatlon("irregular_latlon"); // Name is what is returned by grib_api
