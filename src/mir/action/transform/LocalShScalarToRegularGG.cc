/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/action/transform/LocalShScalarToRegularGG.h"

#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/gauss/regular/RegularGG.h"


namespace mir {
namespace action {
namespace transform {


LocalShScalarToRegularGG::LocalShScalarToRegularGG(const param::MIRParametrisation& parametrisation):
    LocalShScalarToGridded(parametrisation) {
    ASSERT(parametrisation_.userParametrisation().get("regular", N_));
}


LocalShScalarToRegularGG::~LocalShScalarToRegularGG() {
}


bool LocalShScalarToRegularGG::sameAs(const Action& other) const {
    const LocalShScalarToRegularGG* o = dynamic_cast<const LocalShScalarToRegularGG*>(&other);
    return o && (N_ == o->N_);
}


void LocalShScalarToRegularGG::print(std::ostream& out) const {
    out << "LocalShScalarToRegularGG[N=" << N_ << "]";
}


const char* LocalShScalarToRegularGG::name() const {
    return "LocalShScalarToRegularGG";
}


const repres::Representation* LocalShScalarToRegularGG::outputRepresentation() const {
    return new repres::gauss::regular::RegularGG(N_);
}


void LocalShScalarToRegularGG::setTransOptions(atlas::util::Config& options) const {
    options.set(atlas::option::type("local"));
}


namespace {
static ActionBuilder< LocalShScalarToRegularGG > __action("transform.local-sh-scalar-to-regular-gg");
}


}  // namespace transform
}  // namespace action
}  // namespace mir

