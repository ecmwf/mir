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


namespace mir {
namespace action {
namespace interpolate {


Gridded2ReducedGGPLGiven::Gridded2ReducedGGPLGiven(const param::MIRParametrisation& parametrisation) :
    Gridded2UnrotatedGrid(parametrisation) {
    ASSERT(parametrisation_.userParametrisation().get("pl", pl_));
}


Gridded2ReducedGGPLGiven::~Gridded2ReducedGGPLGiven() = default;


bool Gridded2ReducedGGPLGiven::sameAs(const Action& other) const {
    auto o = dynamic_cast<const Gridded2ReducedGGPLGiven*>(&other);
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


const char* Gridded2ReducedGGPLGiven::name() const {
    return "Gridded2ReducedGGPLGiven";
}


static ActionBuilder<Gridded2ReducedGGPLGiven> grid2grid("interpolate.grid2reduced-gg-pl-given");


}  // namespace interpolate
}  // namespace action
}  // namespace mir
