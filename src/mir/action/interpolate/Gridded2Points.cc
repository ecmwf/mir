// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/action/interpolate/Gridded2Points.h"

#include <ostream>

#include "eckit/geo/grid/Unstructured.h"
#include "eckit/log/JSON.h"

#include "mir/param/MIRParametrisation.h"
#include "mir/repres/UnstructuredGrid.h"
#include "mir/util/Exceptions.h"


namespace mir::action::interpolate {


Gridded2Points::Gridded2Points(const param::MIRParametrisation& param) : Gridded2UnrotatedGrid(param) {
    ASSERT_MSG(parametrisation().userParametrisation().get("latitudes", latitudes_) &&
                   parametrisation().userParametrisation().get("longitudes", longitudes_) && !latitudes_.empty() &&
                   latitudes_.size() == longitudes_.size(),
               "Gridded2Points: requires 'latitudes' and 'longitudes', non-empty and of the same size");
}


bool Gridded2Points::sameAs(const Action& other) const {
    const auto* o = dynamic_cast<const Gridded2Points*>(&other);
    return (o != nullptr) && (latitudes_ == o->latitudes_) && (longitudes_ == o->longitudes_) &&
           Gridded2GriddedInterpolation::sameAs(other);
}


void Gridded2Points::json(eckit::JSON& j) const {
    Gridded2GriddedInterpolation::json(j);
    // the points, by their (unstructured) grid uid
    j << "uid" << eckit::geo::grid::Unstructured::uid_from_latlons(latitudes_, longitudes_);
}


void Gridded2Points::print(std::ostream& out) const {
    out << "Gridded2Points["
           "points="
        << latitudes_.size() << ",";
    Gridded2UnrotatedGrid::print(out);
    out << "]";
}


const repres::Representation* Gridded2Points::outputRepresentation() const {
    return new repres::UnstructuredGrid(latitudes_, longitudes_);
}


const char* Gridded2Points::name() const {
    return "Gridded2Points";
}


static const ActionBuilder<Gridded2Points> grid2grid("interpolate.grid2points");


}  // namespace mir::action::interpolate
