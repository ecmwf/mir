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


#include "mir/action/interpolate/Gridded2RotatedOctahedral.h"

#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/gauss/reduced/RotatedOctahedral.h"


namespace mir {
namespace action {


Gridded2RotatedOctahedral::Gridded2RotatedOctahedral(const param::MIRParametrisation &parametrisation):
    Gridded2GriddedInterpolation(parametrisation) {

    ASSERT(parametrisation_.userParametrisation().get("octahedral", N_));

    std::vector<double> value;
    ASSERT(parametrisation_.userParametrisation().get("rotation", value));
    ASSERT(value.size() == 2);

    rotation_ = util::Rotation(value[0], value[1]);
}


Gridded2RotatedOctahedral::~Gridded2RotatedOctahedral() {
}


bool Gridded2RotatedOctahedral::sameAs(const Action& other) const {
    const Gridded2RotatedOctahedral* o = dynamic_cast<const Gridded2RotatedOctahedral*>(&other);
    return o && (N_ == o->N_) && (rotation_ == o->rotation_) && Gridded2GriddedInterpolation::sameAs(other);
}

void Gridded2RotatedOctahedral::print(std::ostream &out) const {
    out << "Gridded2RotatedOctahedral[N="
        << N_
        << ",rotation="
        << rotation_
        << ",";
    Gridded2GriddedInterpolation::print(out);
    out << "]";
}


const repres::Representation *Gridded2RotatedOctahedral::outputRepresentation() const {
    return new repres::gauss::reduced::RotatedOctahedral(N_, rotation_);
}


const char* Gridded2RotatedOctahedral::name() const {
    return "Gridded2RotatedOctahedral";
}


namespace {
static ActionBuilder< Gridded2RotatedOctahedral > grid2grid("interpolate.grid2rotated-octahedral-gg");
}


}  // namespace action
}  // namespace mir

