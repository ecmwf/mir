/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/action/transform/LocalShScalarToReducedGG.h"

#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/gauss/reduced/ReducedClassic.h"


namespace mir {
namespace action {
namespace transform {


LocalShScalarToReducedGG::LocalShScalarToReducedGG(const param::MIRParametrisation& parametrisation):
    LocalShScalarToGridded(parametrisation) {

    ASSERT(parametrisation_.userParametrisation().get("reduced", N_));

}


LocalShScalarToReducedGG::~LocalShScalarToReducedGG() {
}


bool LocalShScalarToReducedGG::sameAs(const Action& other) const {
    const LocalShScalarToReducedGG* o = dynamic_cast<const LocalShScalarToReducedGG*>(&other);
    return o && (N_ == o->N_);
}


void LocalShScalarToReducedGG::print(std::ostream& out) const {
    out << "LocalShScalarToReducedGG[N=" << N_ << "]";
}


const char* LocalShScalarToReducedGG::name() const {
    return "LocalShScalarToReducedGG";
}


const repres::Representation* LocalShScalarToReducedGG::outputRepresentation() const {
    return new repres::gauss::reduced::ReducedClassic(N_);
}


void LocalShScalarToReducedGG::setTransOptions(atlas::util::Config& options) const {
    options.set(atlas::option::type("local"));
}


namespace {
static ActionBuilder< LocalShScalarToReducedGG > __action("transform.local-sh-scalar-to-reduced-gg");
}


}  // namespace transform
}  // namespace action
}  // namespace mir

