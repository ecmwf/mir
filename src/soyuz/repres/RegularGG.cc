/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date Apr 2015

#include "soyuz/repres/RegularGG.h"

#include <iostream>

#include "eckit/exception/Exceptions.h"

#include "soyuz/param/MIRParametrisation.h"



#include "atlas/Grid.h"
#include "atlas/GridSpec.h"
#include "atlas/grids/GaussianGrid.h"

namespace mir {
namespace repres {


RegularGG::RegularGG(const param::MIRParametrisation &parametrisation) {
    eckit::Translator<std::string, int> s2i;
    std::string value;

    ASSERT(parametrisation.get("N", value));
    n_ = s2i(value);
}

RegularGG::RegularGG(int n):
    n_(n) {
}

RegularGG::RegularGG() {
}


RegularGG::~RegularGG() {
}


void RegularGG::print(std::ostream &out) const {
    out << "RegularGG["
        << "]";
}


void RegularGG::fill(grib_info &info) const  {
    NOTIMP;
}

atlas::Grid* RegularGG::atlasGrid() const {
    // TODO: Don't jump in hoops like that
    atlas::Grid *g = atlas::Grid::create(
                         atlas::grids::GaussianGrid(n_).spec()
                     );
    return g;
}

namespace {
static RepresentationBuilder<RegularGG> regularGG("regular_gg"); // Name is what is returned by grib_api
}


}  // namespace repres
}  // namespace mir

