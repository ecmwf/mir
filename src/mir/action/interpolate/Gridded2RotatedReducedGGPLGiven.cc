/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/action/interpolate/Gridded2RotatedReducedGGPLGiven.h"

#include <iostream>
#include <vector>
#include "eckit/exception/Exceptions.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/gauss/reduced/RotatedFromPL.h"


namespace mir {
namespace action {
namespace interpolate {


Gridded2RotatedReducedGGPLGiven::Gridded2RotatedReducedGGPLGiven(const param::MIRParametrisation& parametrisation):
    Gridded2GriddedInterpolation(parametrisation) {
    ASSERT(parametrisation_.userParametrisation().get("pl", pl_));

    std::vector<double> value;
    ASSERT(parametrisation_.userParametrisation().get("rotation", value));
    ASSERT(value.size() == 2);

    rotation_ = util::Rotation(value[0], value[1]);
}


Gridded2RotatedReducedGGPLGiven::~Gridded2RotatedReducedGGPLGiven() {
}


bool Gridded2RotatedReducedGGPLGiven::sameAs(const Action& other) const {
    const Gridded2RotatedReducedGGPLGiven* o = dynamic_cast<const Gridded2RotatedReducedGGPLGiven*>(&other);
    return o && (pl_ == o->pl_) && (rotation_ == o->rotation_);
}


void Gridded2RotatedReducedGGPLGiven::print(std::ostream& out) const {
    out << "Gridded2RotatedReducedGGPLGiven["
        "pl=" << pl_.size()
        << ",rotation="
        << rotation_
        << ",";
    Gridded2GriddedInterpolation::print(out);
    out << "]";
}


const repres::Representation* Gridded2RotatedReducedGGPLGiven::outputRepresentation() const {
    return new repres::gauss::reduced::RotatedFromPL(pl_, rotation_);
}

const char* Gridded2RotatedReducedGGPLGiven::name() const {
    return "Gridded2RotatedReducedGGPLGiven";
}


namespace {
static ActionBuilder< Gridded2RotatedReducedGGPLGiven > grid2grid("interpolate.grid2rotated-reduced-gg-pl-given");
}


}  // namespace interpolate
}  // namespace action
}  // namespace mir

