/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/action/transform/LocalShVodTouvOctahedralGG.h"

#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/gauss/reduced/ReducedOctahedral.h"


namespace mir {
namespace action {
namespace transform {


LocalShVodTouvOctahedralGG::LocalShVodTouvOctahedralGG(const param::MIRParametrisation& parametrisation):
    LocalShVodTouvGridded(parametrisation) {

    ASSERT(parametrisation_.userParametrisation().get("octahedral", N_));

}


bool LocalShVodTouvOctahedralGG::sameAs(const Action& other) const {
    const LocalShVodTouvOctahedralGG* o = dynamic_cast<const LocalShVodTouvOctahedralGG*>(&other);
    return o && (N_ == o->N_);
}


LocalShVodTouvOctahedralGG::~LocalShVodTouvOctahedralGG() {
}


void LocalShVodTouvOctahedralGG::print(std::ostream& out) const {
    out << "LocalShVodTouvOctahedralGG[N=" << N_ << "]";
}


const char* LocalShVodTouvOctahedralGG::name() const {
    return "LocalShVodTouvOctahedralGG";
}


const repres::Representation* LocalShVodTouvOctahedralGG::outputRepresentation() const {
    return new repres::gauss::reduced::ReducedOctahedral(N_);
}


void LocalShVodTouvOctahedralGG::setTransOptions(atlas::util::Config& options) const {
    options.set(atlas::option::type("local"));
}


namespace {
static ActionBuilder< LocalShVodTouvOctahedralGG > __action("transform.local-sh-vod-to-uv-octahedral-gg");
}


}  // namespace transform
}  // namespace action
}  // namespace mir

