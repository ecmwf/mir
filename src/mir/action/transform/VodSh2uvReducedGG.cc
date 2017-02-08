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


#include "mir/action/transform/VodSh2uvReducedGG.h"

#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/gauss/reduced/ReducedClassic.h"


namespace mir {
namespace action {
namespace transform {


VodSh2uvReducedGG::VodSh2uvReducedGG(const param::MIRParametrisation& parametrisation):
    VodSh2uvGridded(parametrisation) {

    ASSERT(parametrisation_.get("user.reduced", N_));

}


VodSh2uvReducedGG::~VodSh2uvReducedGG() {
}


bool VodSh2uvReducedGG::sameAs(const Action& other) const {
    const VodSh2uvReducedGG* o = dynamic_cast<const VodSh2uvReducedGG*>(&other);
    return o && (N_ == o->N_);
}


void VodSh2uvReducedGG::print(std::ostream& out) const {
    out << "VodSh2uvReducedGG[N=" << N_ << "]";
}


const repres::Representation* VodSh2uvReducedGG::outputRepresentation() const {
    return new repres::reduced::ReducedClassic(N_);
}


namespace {
static ActionBuilder< VodSh2uvReducedGG > __action("transform.vod-sh-to-uv-reduced-gg");
}


}  // namespace transform
}  // namespace action
}  // namespace mir

