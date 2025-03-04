/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/action/interpolate/Gridded2RotatedOctahedralGG.h"

#include <ostream>

#include "mir/param/MIRParametrisation.h"
#include "mir/repres/gauss/reduced/RotatedOctahedral.h"
#include "mir/util/Exceptions.h"


namespace mir::action::interpolate {


Gridded2RotatedOctahedralGG::Gridded2RotatedOctahedralGG(const param::MIRParametrisation& param) :
    Gridded2RotatedGrid(param), N_(0) {
    ASSERT(parametrisation().userParametrisation().get("octahedral", N_));
    ASSERT(N_ > 0);
}


bool Gridded2RotatedOctahedralGG::sameAs(const Action& other) const {
    const auto* o = dynamic_cast<const Gridded2RotatedOctahedralGG*>(&other);
    return (o != nullptr) && (N_ == o->N_) && Gridded2RotatedGrid::sameAs(other);
}


void Gridded2RotatedOctahedralGG::print(std::ostream& out) const {
    out << "Gridded2RotatedOctahedralGG["
           "N="
        << N_
        << ","
           "rotation="
        << rotation() << ",";
    Gridded2RotatedGrid::print(out);
    out << "]";
}


const repres::Representation* Gridded2RotatedOctahedralGG::outputRepresentation() const {
    return new repres::gauss::reduced::RotatedOctahedral(N_, rotation());
}


const char* Gridded2RotatedOctahedralGG::name() const {
    return "Gridded2RotatedOctahedralGG";
}


static const ActionBuilder<Gridded2RotatedOctahedralGG> grid2grid("interpolate.grid2rotated-octahedral-gg");


}  // namespace mir::action::interpolate
