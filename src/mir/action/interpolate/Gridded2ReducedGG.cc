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


#include "mir/action/interpolate/Gridded2ReducedGG.h"

#include <ostream>

#include "mir/param/MIRParametrisation.h"
#include "mir/repres/gauss/reduced/ReducedClassic.h"
#include "mir/util/Exceptions.h"


namespace mir::action::interpolate {


Gridded2ReducedGG::Gridded2ReducedGG(const param::MIRParametrisation& param) : Gridded2UnrotatedGrid(param) {
    ASSERT(parametrisation().userParametrisation().get("reduced", N_));
}


bool Gridded2ReducedGG::sameAs(const Action& other) const {
    const auto* o = dynamic_cast<const Gridded2ReducedGG*>(&other);
    return (o != nullptr) && (N_ == o->N_) && Gridded2GriddedInterpolation::sameAs(other);
}


void Gridded2ReducedGG::print(std::ostream& out) const {
    out << "Gridded2ReducedGG["
           "N="
        << N_ << ",";
    Gridded2UnrotatedGrid::print(out);
    out << "]";
}


const repres::Representation* Gridded2ReducedGG::outputRepresentation() const {
    return new repres::gauss::reduced::ReducedClassic(N_);
}


const char* Gridded2ReducedGG::name() const {
    return "Gridded2ReducedGG";
}


static const ActionBuilder<Gridded2ReducedGG> grid2grid("interpolate.grid2reduced-gg");


}  // namespace mir::action::interpolate
