
// File TriangularGrid.cc
// Baudouin Raoult - (c) ECMWF Apr 15

#include "TriangularGrid.h"
#include "MIRParametrisation.h"
#include "eckit/exception/Exceptions.h"

#include <iostream>

TriangularGrid::TriangularGrid(const MIRParametrisation &parametrisation) {
}

TriangularGrid::TriangularGrid()
{
}


TriangularGrid::~TriangularGrid() {
}


void TriangularGrid::print(std::ostream &out) const {
    out << "TriangularGrid["
        << "]";
}


void TriangularGrid::fill(grib_spec &spec) const  {
    NOTIMP;
}



static RepresentationBuilder<TriangularGrid> triangularGrid("triangular_grid"); // Name is what is returned by grib_api
