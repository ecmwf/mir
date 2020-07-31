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


#include "mir/action/interpolate/Gridded2TypedGrid.h"

#include <iostream>

#include "eckit/exception/Exceptions.h"
#include "eckit/utils/StringTools.h"

#include "mir/config/LibMir.h"
#include "mir/key/grid/Grid.h"
#include "mir/repres/Representation.h"


namespace mir {
namespace action {
namespace interpolate {


Gridded2TypedGrid::Gridded2TypedGrid(const param::MIRParametrisation& parametrisation) :
    Gridded2UnrotatedGrid(parametrisation) {

    std::string grid;
    ASSERT(parametrisation_.userParametrisation().get("grid", grid));

    for (auto kv_str : eckit::StringTools::split(",", grid)) {
        auto kv = eckit::StringTools::split("=", kv_str);
        if (kv.size() != 2) {
            throw eckit::UserError("Gridded2TypedGrid: invalid key=value pair, got '" + kv_str + "'");
        }
        gridParams_.set(kv[0], kv[1]);
    }

    std::string gridType;
    if (!gridParams_.get("gridType", gridType)) {
        throw eckit::UserError("Gridded2TypedGrid: expecting key 'gridType'");
    }
}


Gridded2TypedGrid::~Gridded2TypedGrid() = default;


bool Gridded2TypedGrid::sameAs(const Action& other) const {
    auto o = dynamic_cast<const Gridded2TypedGrid*>(&other);

    // NOTE: SimpleParametrisation::matches() is not commutative
    return (o != nullptr) && gridParams_.matches(o->gridParams_) && o->gridParams_.matches(gridParams_) &&
           Gridded2GriddedInterpolation::sameAs(other);
}


void Gridded2TypedGrid::print(std::ostream& out) const {
    out << "Gridded2TypedGrid[grid=" << gridParams_ << ",";
    Gridded2UnrotatedGrid::print(out);
    out << "]";
}


const repres::Representation* Gridded2TypedGrid::outputRepresentation() const {
    std::string gridType;
    ASSERT(gridParams_.get("gridType", gridType));

    const auto& g = key::grid::Grid::lookup(gridType);
    return g.representation(gridParams_);
}


const char* Gridded2TypedGrid::name() const {
    return "Gridded2TypedGrid";
}


static ActionBuilder<Gridded2TypedGrid> grid2grid("interpolate.grid2typedgrid");


}  // namespace interpolate
}  // namespace action
}  // namespace mir
