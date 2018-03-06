/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/action/transform/LocalShScalarToOctahedralGG.h"

#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/gauss/reduced/ReducedOctahedral.h"


namespace mir {
namespace action {
namespace transform {


LocalShScalarToOctahedralGG::LocalShScalarToOctahedralGG(const param::MIRParametrisation& parametrisation):
    LocalShScalarToGridded(parametrisation) {

    ASSERT(parametrisation_.userParametrisation().get("octahedral", N_));

}


bool LocalShScalarToOctahedralGG::sameAs(const Action& other) const {
    const LocalShScalarToOctahedralGG* o = dynamic_cast<const LocalShScalarToOctahedralGG*>(&other);
    return o && (N_ == o->N_);
}


LocalShScalarToOctahedralGG::~LocalShScalarToOctahedralGG() {
}


void LocalShScalarToOctahedralGG::print(std::ostream& out) const {
    out << "LocalShScalarToOctahedralGG[N=" << N_ << "]";
}


const char* LocalShScalarToOctahedralGG::name() const {
    return "LocalShScalarToOctahedralGG";
}


const repres::Representation* LocalShScalarToOctahedralGG::outputRepresentation() const {
    return new repres::gauss::reduced::ReducedOctahedral(N_);
}


void LocalShScalarToOctahedralGG::setTransOptions(atlas::util::Config& options) const {
    options.set(atlas::option::type("local"));
}


namespace {
static ActionBuilder< LocalShScalarToOctahedralGG > __action("transform.local-sh-scalar-to-octahedral-gg");
}


}  // namespace transform
}  // namespace action
}  // namespace mir

