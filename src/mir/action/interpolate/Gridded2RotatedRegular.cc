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


#include "mir/action/interpolate/Gridded2RotatedRegular.h"

#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/gauss/regular/RotatedGG.h"


namespace mir {
namespace action {


Gridded2RotatedRegular::Gridded2RotatedRegular(const param::MIRParametrisation &parametrisation):
    Gridded2GriddedInterpolation(parametrisation) {

    ASSERT(parametrisation_.userParametrisation().get("regular", N_));

    std::vector<double> value;
    ASSERT(parametrisation_.userParametrisation().get("rotation", value));
    ASSERT(value.size() == 2);

    rotation_ = util::Rotation(value[0], value[1]);
}


Gridded2RotatedRegular::~Gridded2RotatedRegular() {
}


bool Gridded2RotatedRegular::sameAs(const Action& other) const {
    const Gridded2RotatedRegular* o = dynamic_cast<const Gridded2RotatedRegular*>(&other);
    return o && (N_ == o->N_) && (rotation_ == o->rotation_) && Gridded2GriddedInterpolation::sameAs(other);
}

void Gridded2RotatedRegular::print(std::ostream &out) const {
    out << "Gridded2RotatedRegular[N="
        << N_
        << ",rotation="
        << rotation_
        << ",";
    Gridded2GriddedInterpolation::print(out);
    out << "]";
}


const repres::Representation *Gridded2RotatedRegular::outputRepresentation() const {
    return new repres::gauss::regular::RotatedGG(N_, util::BoundingBox(), rotation_);
}

const char* Gridded2RotatedRegular::name() const {
    return "Gridded2RotatedRegular";
}

namespace {
static ActionBuilder< Gridded2RotatedRegular > grid2grid("interpolate.grid2rotated-regular-gg");
}


}  // namespace action
}  // namespace mir

