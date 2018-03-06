/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/action/transform/LocalShVodTouvRegularGG.h"

#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/gauss/regular/RegularGG.h"


namespace mir {
namespace action {
namespace transform {


LocalShVodTouvRegularGG::LocalShVodTouvRegularGG(const param::MIRParametrisation& parametrisation):
    LocalShVodTouvGridded(parametrisation) {
    ASSERT(parametrisation_.userParametrisation().get("regular", N_));
}


LocalShVodTouvRegularGG::~LocalShVodTouvRegularGG() {
}


bool LocalShVodTouvRegularGG::sameAs(const Action& other) const {
    const LocalShVodTouvRegularGG* o = dynamic_cast<const LocalShVodTouvRegularGG*>(&other);
    return o && (N_ == o->N_);
}


void LocalShVodTouvRegularGG::print(std::ostream& out) const {
    out << "LocalShVodTouvRegularGG[N=" << N_ << "]";
}


const char* LocalShVodTouvRegularGG::name() const {
    return "LocalShVodTouvRegularGG";
}


const repres::Representation* LocalShVodTouvRegularGG::outputRepresentation() const {
    return new repres::gauss::regular::RegularGG(N_);
}


void LocalShVodTouvRegularGG::setTransOptions(atlas::util::Config& options) const {
    options.set(atlas::option::type("local"));
}


namespace {
static ActionBuilder< LocalShVodTouvRegularGG > __action("transform.local-sh-vod-to-uv-regular-gg");
}


}  // namespace transform
}  // namespace action
}  // namespace mir

