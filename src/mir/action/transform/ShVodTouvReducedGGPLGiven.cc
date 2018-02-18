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


#include "mir/action/transform/ShVodTouvReducedGGPLGiven.h"

#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/gauss/reduced/ReducedFromPL.h"


namespace mir {
namespace action {
namespace transform {


ShVodTouvReducedGGPLGiven::ShVodTouvReducedGGPLGiven(const param::MIRParametrisation& parametrisation):
    ShVodTouvGridded(parametrisation) {
    ASSERT(parametrisation_.userParametrisation().get("pl", pl_));
}


ShVodTouvReducedGGPLGiven::~ShVodTouvReducedGGPLGiven() {
}


bool ShVodTouvReducedGGPLGiven::sameAs(const Action& other) const {
    const ShVodTouvReducedGGPLGiven* o = dynamic_cast<const ShVodTouvReducedGGPLGiven*>(&other);
    return o && (pl_ == o->pl_);
}


void ShVodTouvReducedGGPLGiven::print(std::ostream& out) const {
    out << "ShVodTouvReducedGGPLGive[pl=" << pl_.size() << "]";
}


const repres::Representation* ShVodTouvReducedGGPLGiven::outputRepresentation() const {
    return new repres::gauss::reduced::ReducedFromPL(pl_);
}

const char* ShVodTouvReducedGGPLGiven::name() const {
    return "ShVodTouvReducedGGPLGiven";
}


namespace {
static ActionBuilder< ShVodTouvReducedGGPLGiven > __action("transform.sh-vod-to-uv-reduced-gg-pl-given");
}


}  // namespace transform
}  // namespace action
}  // namespace mir

