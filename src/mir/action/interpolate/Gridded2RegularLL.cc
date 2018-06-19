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


#include "mir/action/interpolate/Gridded2RegularLL.h"

#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "eckit/types/Fraction.h"
#include "mir/repres/latlon/RegularLL.h"
#include "mir/param/MIRParametrisation.h"


namespace mir {
namespace action {


Gridded2RegularLL::Gridded2RegularLL(const param::MIRParametrisation& parametrisation):
    Gridded2LatLon(parametrisation) {

}


Gridded2RegularLL::~Gridded2RegularLL() = default;


bool Gridded2RegularLL::sameAs(const Action& other) const {
    auto o = dynamic_cast<const Gridded2RegularLL*>(&other);
    return o && Gridded2LatLon::sameAs(*o);
}


void Gridded2RegularLL::print(std::ostream& out) const {
    out << "Gridded2RegularLL[";
    Gridded2LatLon::print(out);
    out << "]";
}


const repres::Representation* Gridded2RegularLL::outputRepresentation() const {
    return new repres::latlon::RegularLL(bbox_, increments_);
}

const char* Gridded2RegularLL::name() const {
    return "Gridded2RegularLL";
}


namespace {
static ActionBuilder< Gridded2RegularLL > grid2grid("interpolate.grid2regular-ll");
}


}  // namespace action
}  // namespace mir

