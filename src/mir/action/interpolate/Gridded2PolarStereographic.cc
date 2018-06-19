/*
 * (C) Copyright 1996- ECMWF.
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

#include "mir/action/interpolate/Gridded2PolarStereographic.h"

#include <iostream>

#include "eckit/exception/Exceptions.h"


#include "mir/repres/unsupported/PolarStereographic.h"
#include "mir/param/MIRParametrisation.h"


namespace mir {
namespace action {


Gridded2PolarStereographic::Gridded2PolarStereographic(const param::MIRParametrisation &parametrisation):
    Gridded2GriddedInterpolation(parametrisation) {
    // std::vector<double> value;

    // ASSERT(parametrisation_.user().get("grid", value));
    // ASSERT(value.size() == 2);

    // increments_ = util::Increments(value[0], value[1]);
}


Gridded2PolarStereographic::~Gridded2PolarStereographic() = default;


bool Gridded2PolarStereographic::sameAs(const Action& other) const {
    NOTIMP;
}

void Gridded2PolarStereographic::print(std::ostream &out) const {
    out << "Gridded2PolarStereographic[";
    Gridded2GriddedInterpolation::print(out);
    out << "]";
}


const repres::Representation *Gridded2PolarStereographic::outputRepresentation() const {
    NOTIMP;
}

const char* Gridded2PolarStereographic::name() const {
    return "Gridded2PolarStereographic";
}

namespace {
static ActionBuilder< Gridded2PolarStereographic > grid2grid("interpolate.grid2polar-stereographic");
}


}  // namespace action
}  // namespace mir

