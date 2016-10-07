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


#include "mir/action/interpolate/Gridded2RegularLL.h"

#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "mir/repres/latlon/RegularLL.h"
#include "mir/param/MIRParametrisation.h"


namespace mir {
namespace action {


Gridded2RegularLL::Gridded2RegularLL(const param::MIRParametrisation &parametrisation):
    Gridded2GriddedInterpolation(parametrisation),
    bboxDefinesGrid_(false) {

    std::vector<double> value;
    ASSERT(parametrisation_.get("user.grid", value));
    ASSERT(value.size() == 2);
    increments_ = util::Increments(value[0], value[1]);

    parametrisation_.get("user.bounding-box-defines-grid", bboxDefinesGrid_);
}


Gridded2RegularLL::~Gridded2RegularLL() {
}


bool Gridded2RegularLL::sameAs(const Action& other) const {
    const Gridded2RegularLL* o = dynamic_cast<const Gridded2RegularLL*>(&other);
    return o && (increments_ == o->increments_);
}


void Gridded2RegularLL::print(std::ostream &out) const {
    out << "Gridded2RegularLL[increments=" << increments_ << "]";
}


const repres::Representation *Gridded2RegularLL::outputRepresentation() const {
    double ns = increments_.south_north();
    double we = increments_.west_east();

    // Latitude range: cater for grids that are regular, but do not reach the pole (e.g. 1.6)
    double pole = size_t(90/ns) * ns;

    // Longitude range
    // - periodic grids have East-most longitude at 360 - increment
    // - non-periodic grids are symmetric to Greenwhich and do not reach the date line (e.g. 1.1)
    double west = 0;
    double east = size_t(360/we) * we;
    if (east == 360) {
        east -= we;
    }
    else {
        east = size_t(180/we) * we;
        west = -east;
    }

    return new repres::latlon::RegularLL(
                util::BoundingBox(pole, west, -pole, east),
                increments_,
                bboxDefinesGrid_ );
}


namespace {
static ActionBuilder< Gridded2RegularLL > grid2grid("interpolate.grid2regular-ll");
}


}  // namespace action
}  // namespace mir

