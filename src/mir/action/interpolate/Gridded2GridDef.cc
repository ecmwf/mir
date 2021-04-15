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


#include "mir/action/interpolate/Gridded2GridDef.h"

#include <ostream>

#include "eckit/filesystem/PathName.h"

#include "mir/param/MIRParametrisation.h"
#include "mir/repres/other/UnstructuredGrid.h"
#include "mir/util/Exceptions.h"


namespace mir {
namespace action {
namespace interpolate {


Gridded2GridDef::Gridded2GridDef(const param::MIRParametrisation& parametrisation) :
    Gridded2UnrotatedGrid(parametrisation) {
    ASSERT(parametrisation_.userParametrisation().get("griddef", griddef_));
}


Gridded2GridDef::~Gridded2GridDef() = default;


bool Gridded2GridDef::sameAs(const Action& other) const {
    auto o = dynamic_cast<const Gridded2GridDef*>(&other);
    return (o != nullptr) && (griddef_ == o->griddef_) && Gridded2GriddedInterpolation::sameAs(other);
}


void Gridded2GridDef::custom(std::ostream& out) const {
    out << "Gridded2GridDef["
           "griddef=.../"
        << eckit::PathName(griddef_).baseName() << "]";
}


void Gridded2GridDef::print(std::ostream& out) const {
    out << "Gridded2GridDef["
        << "griddef=" << griddef_ << ",";
    Gridded2UnrotatedGrid::print(out);
    out << "]";
}


const repres::Representation* Gridded2GridDef::outputRepresentation() const {
    return new repres::other::UnstructuredGrid(griddef_);
}


const char* Gridded2GridDef::name() const {
    return "Gridded2GridDef";
}


static ActionBuilder<Gridded2GridDef> grid2grid("interpolate.grid2griddef");


}  // namespace interpolate
}  // namespace action
}  // namespace mir
