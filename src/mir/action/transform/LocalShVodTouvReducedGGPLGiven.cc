/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/action/transform/LocalShVodTouvReducedGGPLGiven.h"

#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/gauss/reduced/ReducedFromPL.h"


namespace mir {
namespace action {
namespace transform {


LocalShVodTouvReducedGGPLGiven::LocalShVodTouvReducedGGPLGiven(const param::MIRParametrisation& parametrisation):
    LocalShVodTouvGridded(parametrisation) {
    ASSERT(parametrisation_.userParametrisation().get("pl", pl_));
}


LocalShVodTouvReducedGGPLGiven::~LocalShVodTouvReducedGGPLGiven() {
}


bool LocalShVodTouvReducedGGPLGiven::sameAs(const Action& other) const {
    const LocalShVodTouvReducedGGPLGiven* o = dynamic_cast<const LocalShVodTouvReducedGGPLGiven*>(&other);
    return o && (pl_ == o->pl_);
}


void LocalShVodTouvReducedGGPLGiven::print(std::ostream& out) const {
    out << "LocalShVodTouvReducedGGPLGive[pl=" << pl_.size() << "]";
}


const char* LocalShVodTouvReducedGGPLGiven::name() const {
    return "LocalShVodTouvReducedGGPLGiven";
}


const repres::Representation* LocalShVodTouvReducedGGPLGiven::outputRepresentation() const {
    return new repres::gauss::reduced::ReducedFromPL(pl_);
}


void LocalShVodTouvReducedGGPLGiven::setTransOptions(atlas::util::Config& options) const {
    options.set(atlas::option::type("local"));
}


namespace {
static ActionBuilder< LocalShVodTouvReducedGGPLGiven > __action("transform.local-sh-vod-to-uv-reduced-gg-pl-given");
}


}  // namespace transform
}  // namespace action
}  // namespace mir

