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


#include "mir/action/interpolate/Gridded2GridSpec.h"

#include <ostream>

#include "eckit/geo/Grid.h"

#include "mir/key/grid/Grid.h"
#include "mir/repres/Representation.h"
#include "mir/util/Exceptions.h"


namespace mir::action::interpolate {


Gridded2GridSpec::Gridded2GridSpec(const param::MIRParametrisation& param) : Gridded2UnrotatedGrid(param) {
    // assign grid
    std::string gridspec;
    ASSERT(key::grid::Grid::get("grid", gridspec, param));

    // assign compatible parametrisation
    param_ = std::make_unique<param::GridSpecParametrisation>(eckit::geo::GridFactory::make_from_string(gridspec));
    ASSERT(param_);
}


bool Gridded2GridSpec::sameAs(const Action& other) const {
    const auto* o = dynamic_cast<const Gridded2GridSpec*>(&other);
    return (o != nullptr) && (param_->spec().str() == o->param_->spec().str()) &&
           Gridded2GriddedInterpolation::sameAs(other);
}


void Gridded2GridSpec::print(std::ostream& out) const {
    out << "Gridded2GridSpec[gridspec=" << param_->spec() << ",";
    Gridded2UnrotatedGrid::print(out);
    out << "]";
}


const char* Gridded2GridSpec::name() const {
    return "Gridded2GridSpec";
}


const repres::Representation* Gridded2GridSpec::outputRepresentation() const {
    return repres::RepresentationFactory::build(*param_);
}


static const ActionBuilder<Gridded2GridSpec> grid2grid("interpolate.grid2gridspec");


}  // namespace mir::action::interpolate
