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


#include "mir/action/interpolate/Gridded2OctahedralGG.h"

#include <ostream>

#include "mir/param/MIRParametrisation.h"
#include "mir/repres/gauss/reduced/ReducedOctahedral.h"
#include "mir/util/Exceptions.h"


namespace mir::action::interpolate {


Gridded2OctahedralGG::Gridded2OctahedralGG(const param::MIRParametrisation& param) : Gridded2UnrotatedGrid(param) {
    ASSERT(parametrisation().userParametrisation().get("octahedral", N_));
}


bool Gridded2OctahedralGG::sameAs(const Action& other) const {
    const auto* o = dynamic_cast<const Gridded2OctahedralGG*>(&other);
    return (o != nullptr) && (N_ == o->N_) && Gridded2GriddedInterpolation::sameAs(other);
}


void Gridded2OctahedralGG::print(std::ostream& out) const {
    out << "Gridded2OctahedralGG["
           "N="
        << N_ << ",";
    Gridded2UnrotatedGrid::print(out);
    out << "]";
}


const repres::Representation* Gridded2OctahedralGG::outputRepresentation() const {
    return new repres::gauss::reduced::ReducedOctahedral(N_);
}


const char* Gridded2OctahedralGG::name() const {
    return "Gridded2OctahedralGG";
}


static const ActionBuilder<Gridded2OctahedralGG> grid2grid("interpolate.grid2octahedral-gg");


}  // namespace mir::action::interpolate
