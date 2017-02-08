/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @date Feb 2017


#include "mir/action/transform/VodSh2uvRegularGG.h"

#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/gauss/regular/RegularGG.h"


namespace mir {
namespace action {
namespace transform {


VodSh2uvRegularGG::VodSh2uvRegularGG(const param::MIRParametrisation& parametrisation):
    VodSh2uvGridded(parametrisation) {
    ASSERT(parametrisation_.get("user.regular", N_));
}


VodSh2uvRegularGG::~VodSh2uvRegularGG() {
}


bool VodSh2uvRegularGG::sameAs(const Action& other) const {
    const VodSh2uvRegularGG* o = dynamic_cast<const VodSh2uvRegularGG*>(&other);
    return o && (N_ == o->N_);
}


void VodSh2uvRegularGG::print(std::ostream& out) const {
    out << "VodSh2uvRegularGG[N=" << N_ << "]";
}


const repres::Representation* VodSh2uvRegularGG::outputRepresentation() const {
    return new repres::regular::RegularGG(N_);
}


namespace {
static ActionBuilder< VodSh2uvRegularGG > __action("transform.vod-sh-to-uv-regular-gg");
}


}  // namespace transform
}  // namespace action
}  // namespace mir

