/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/action/transform/LocalShVodTouvReducedGG.h"

#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/gauss/reduced/ReducedClassic.h"


namespace mir {
namespace action {
namespace transform {


LocalShVodTouvReducedGG::LocalShVodTouvReducedGG(const param::MIRParametrisation& parametrisation):
    LocalShVodTouvGridded(parametrisation) {

    ASSERT(parametrisation_.userParametrisation().get("reduced", N_));

}


LocalShVodTouvReducedGG::~LocalShVodTouvReducedGG() {
}


bool LocalShVodTouvReducedGG::sameAs(const Action& other) const {
    const LocalShVodTouvReducedGG* o = dynamic_cast<const LocalShVodTouvReducedGG*>(&other);
    return o && (N_ == o->N_);
}


void LocalShVodTouvReducedGG::print(std::ostream& out) const {
    out << "LocalShVodTouvReducedGG[N=" << N_ << "]";
}


const char* LocalShVodTouvReducedGG::name() const {
    return "LocalShVodTouvReducedGG";
}


const repres::Representation* LocalShVodTouvReducedGG::outputRepresentation() const {
    return new repres::gauss::reduced::ReducedClassic(N_);
}


void LocalShVodTouvReducedGG::setTransOptions(atlas::util::Config& options) const {
    options.set(atlas::option::type("local"));
}


namespace {
static ActionBuilder< LocalShVodTouvReducedGG > __action("transform.local-sh-vod-to-uv-reduced-gg");
}


}  // namespace transform
}  // namespace action
}  // namespace mir

