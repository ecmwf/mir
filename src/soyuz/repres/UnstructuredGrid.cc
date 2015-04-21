
// File UnstructuredGrid.cc
// Baudouin Raoult - (c) ECMWF Apr 15

#include "soyuz/repres/UnstructuredGrid.h"
#include "soyuz/param/MIRParametrisation.h"
#include "eckit/exception/Exceptions.h"

#include <iostream>

UnstructuredGrid::UnstructuredGrid(const MIRParametrisation &parametrisation) {
}

UnstructuredGrid::UnstructuredGrid() {
}


UnstructuredGrid::~UnstructuredGrid() {
}


void UnstructuredGrid::print(std::ostream &out) const {
    out << "UnstructuredGrid["
        << "]";
}


void UnstructuredGrid::fill(grib_info &info) const  {
    NOTIMP;
}



static RepresentationBuilder<UnstructuredGrid> unstructuredGrid("unstructured_grid"); // Name is what is returned by grib_api
