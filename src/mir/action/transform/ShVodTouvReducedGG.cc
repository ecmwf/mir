/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @date Feb 2017


#include "mir/action/transform/ShVodTouvReducedGG.h"

#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/gauss/reduced/ReducedClassic.h"


namespace mir {
namespace action {
namespace transform {


ShVodTouvReducedGG::ShVodTouvReducedGG(const param::MIRParametrisation& parametrisation):
    ShVodTouvGridded(parametrisation) {

    ASSERT(parametrisation_.userParametrisation().get("reduced", N_));

}


ShVodTouvReducedGG::~ShVodTouvReducedGG() {
}


bool ShVodTouvReducedGG::sameAs(const Action& other) const {
    const ShVodTouvReducedGG* o = dynamic_cast<const ShVodTouvReducedGG*>(&other);
    return o && (N_ == o->N_);
}


void ShVodTouvReducedGG::print(std::ostream& out) const {
    out << "ShVodTouvReducedGG[";
    ShToGridded::print(out);
    out << ",N=" << N_
        << "]";
}


const char* ShVodTouvReducedGG::name() const {
    return "ShVodTouvReducedGG";
}


const repres::Representation* ShVodTouvReducedGG::outputRepresentation() const {
    return new repres::gauss::reduced::ReducedClassic(N_);
}


namespace {
static ActionBuilder< ShVodTouvReducedGG > __action("transform.sh-vod-to-uv-reduced-gg");
}


}  // namespace transform
}  // namespace action
}  // namespace mir

