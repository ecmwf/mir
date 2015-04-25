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

#include "soyuz/repres/ReducedGG.h"


#include <iostream>

#include "eckit/exception/Exceptions.h"

#include "soyuz/param/MIRParametrisation.h"


#include "atlas/Grid.h"
// #include "atlas/GridSpec.h"
#include "atlas/grids/ReducedGaussianGrid.h"
#include <eckit/parser/Tokenizer.h>

#include "atlas/grids/grids.h"

namespace mir {
namespace repres {


ReducedGG::ReducedGG(const param::MIRParametrisation &parametrisation) {
    eckit::Translator<std::string, int> s2i;
    std::string value;

    ASSERT(parametrisation.get("N", value));
    n_ = s2i(value);

    // FIXME: Not the most efficient

    ASSERT(parametrisation.get("pl", value));

    eckit::Tokenizer parse("/");
    std::vector<std::string> pl;
    parse(value, pl);

    pl_.reserve(pl.size());
    for (size_t i = 0; i < pl.size(); i++) {
        pl_.push_back(s2i(pl[i]));
    }

}

ReducedGG::ReducedGG(int n):
    n_(n) {

}

ReducedGG::ReducedGG() {
}


ReducedGG::~ReducedGG() {
}


void ReducedGG::print(std::ostream &out) const {
    out << "ReducedGG["
        << "]";
}


void ReducedGG::fill(grib_info &info) const  {
    NOTIMP;
}

atlas::Grid *ReducedGG::atlasGrid() const {
    if (pl_.size() > 0) {
        return new atlas::grids::ReducedGaussianGrid(n_, &pl_[0]);
    } else {
#if 0
        eckit::StrStream os;
        os << "reduced_gg.N" << n_ << eckit::StrStream::ends;
        return atlas::Grid::create(std::string(os));
#else

        switch (n_) {
        case 32:
            return new atlas::grids::rgg::N32() ;
            break;

        case 48:
            return new atlas::grids::rgg::N48() ;
            break;

        case 80:
            return new atlas::grids::rgg::N80() ;
            break;

        case 128:
            return new atlas::grids::rgg::N128() ;
            break;

        case 256:
            return new atlas::grids::rgg::N256() ;
            break;

        case 640:
            return new atlas::grids::rgg::N640() ;
            break;

        case 2000:
            return new atlas::grids::rgg::N2000() ;
            break;

        case 4000:
            return new atlas::grids::rgg::N4000() ;
            break;
        }

        eckit::StrStream os;
        os << "Unsupported reduced Gaussian grid: N" << n_ << eckit::StrStream::ends;
        throw eckit::SeriousBug(os);
#endif

    }
}

namespace {
static RepresentationBuilder<ReducedGG> reducedGG("reduced_gg"); // Name is what is returned by grib_api
}


}  // namespace repres
}  // namespace mir

