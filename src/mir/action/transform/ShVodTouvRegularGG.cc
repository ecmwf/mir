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


#include "mir/action/transform/ShVodTouvRegularGG.h"

#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/gauss/regular/RegularGG.h"


namespace mir {
namespace action {
namespace transform {


ShVodTouvRegularGG::ShVodTouvRegularGG(const param::MIRParametrisation& parametrisation):
    ShVodTouvGridded(parametrisation) {
    ASSERT(parametrisation_.userParametrisation().get("regular", N_));
}


ShVodTouvRegularGG::~ShVodTouvRegularGG() {
}


bool ShVodTouvRegularGG::sameAs(const Action& other) const {
    const ShVodTouvRegularGG* o = dynamic_cast<const ShVodTouvRegularGG*>(&other);
    return o && (N_ == o->N_);
}


void ShVodTouvRegularGG::print(std::ostream& out) const {
    out << "ShVodTouvRegularGG[N=" << N_ << "]";
}


const char* ShVodTouvRegularGG::name() const {
    return "ShVodTouvRegularGG";
}


const repres::Representation* ShVodTouvRegularGG::outputRepresentation() const {
    return new repres::gauss::regular::RegularGG(N_);
}


void ShVodTouvRegularGG::setTransOptions(atlas::util::Config& options) const {
    options.set(atlas::option::type("ifs"));
}


namespace {
static ActionBuilder< ShVodTouvRegularGG > __action("transform.sh-vod-to-uv-regular-gg");
}


}  // namespace transform
}  // namespace action
}  // namespace mir

