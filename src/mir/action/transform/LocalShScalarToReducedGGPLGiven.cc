/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/action/transform/LocalShScalarToReducedGGPLGiven.h"

#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/gauss/reduced/ReducedFromPL.h"


namespace mir {
namespace action {
namespace transform {


LocalShScalarToReducedGGPLGiven::LocalShScalarToReducedGGPLGiven(const param::MIRParametrisation& parametrisation):
    LocalShScalarToGridded(parametrisation) {
    ASSERT(parametrisation_.userParametrisation().get("pl", pl_));
}


LocalShScalarToReducedGGPLGiven::~LocalShScalarToReducedGGPLGiven() {
}


bool LocalShScalarToReducedGGPLGiven::sameAs(const Action& other) const {
    const LocalShScalarToReducedGGPLGiven* o = dynamic_cast<const LocalShScalarToReducedGGPLGiven*>(&other);
    return o && (pl_ == o->pl_);
}


void LocalShScalarToReducedGGPLGiven::print(std::ostream& out) const {
    out << "LocalShScalarToReducedGGPLGive[pl=" << pl_.size() << "]";
}


const char* LocalShScalarToReducedGGPLGiven::name() const {
    return "LocalShScalarToReducedGGPLGiven";
}


const repres::Representation* LocalShScalarToReducedGGPLGiven::outputRepresentation() const {
    return new repres::gauss::reduced::ReducedFromPL(pl_);
}


void LocalShScalarToReducedGGPLGiven::setTransOptions(atlas::util::Config& options) const {
    options.set(atlas::option::type("local"));
}


namespace {
static ActionBuilder< LocalShScalarToReducedGGPLGiven > __action("transform.local-sh-scalar-to-reduced-gg-pl-given");
}


}  // namespace transform
}  // namespace action
}  // namespace mir

