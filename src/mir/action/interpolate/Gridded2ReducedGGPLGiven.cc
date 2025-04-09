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


#include "mir/action/interpolate/Gridded2ReducedGGPLGiven.h"

#include <ostream>

#include "mir/param/MIRParametrisation.h"
#include "mir/repres/gauss/reduced/ReducedFromPL.h"
#include "mir/util/Exceptions.h"


namespace mir::action::interpolate {


Gridded2ReducedGGPLGiven::Gridded2ReducedGGPLGiven(const param::MIRParametrisation& param) :
    Gridded2UnrotatedGrid(param) {
    ASSERT(parametrisation().userParametrisation().get("pl", pl_));
}


bool Gridded2ReducedGGPLGiven::sameAs(const Action& other) const {
    const auto* o = dynamic_cast<const Gridded2ReducedGGPLGiven*>(&other);
    return (o != nullptr) && (pl_ == o->pl_) && Gridded2GriddedInterpolation::sameAs(other);
}


void Gridded2ReducedGGPLGiven::print(std::ostream& out) const {
    out << "Gridded2ReducedGGPLGiven["
           "pl="
        << pl_.size() << ",";
    Gridded2UnrotatedGrid::print(out);
    out << "]";
}


const repres::Representation* Gridded2ReducedGGPLGiven::outputRepresentation() const {
    size_t N = pl_.size() / 2;
    return new repres::gauss::reduced::ReducedFromPL(N, pl_);
}


bool Gridded2ReducedGGPLGiven::getGriddedTargetName(std::string& name) const {
    name = "N" + std::to_string(pl_.size());
    return true;
}


const char* Gridded2ReducedGGPLGiven::name() const {
    return "Gridded2ReducedGGPLGiven";
}


static const ActionBuilder<Gridded2ReducedGGPLGiven> grid2grid("interpolate.grid2reduced-gg-pl-given");


}  // namespace mir::action::interpolate
