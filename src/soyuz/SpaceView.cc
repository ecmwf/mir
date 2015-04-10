
// File SpaceView.cc
// Baudouin Raoult - (c) ECMWF Apr 15

#include "SpaceView.h"
#include "MIRParametrisation.h"
#include "eckit/exception/Exceptions.h"

#include <iostream>

SpaceView::SpaceView(const MIRParametrisation &parametrisation) {
}

SpaceView::SpaceView()
{
}


SpaceView::~SpaceView() {
}


void SpaceView::print(std::ostream &out) const {
    out << "SpaceView["
        << "]";
}


void SpaceView::fill(grib_info &info) const  {
    NOTIMP;
}



static RepresentationBuilder<SpaceView> spaceView("space_view"); // Name is what is returned by grib_api
