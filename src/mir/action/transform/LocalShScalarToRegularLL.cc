/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/action/transform/LocalShScalarToRegularLL.h"

#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/latlon/RegularLL.h"


namespace mir {
namespace action {
namespace transform {


LocalShScalarToRegularLL::LocalShScalarToRegularLL(const param::MIRParametrisation &parametrisation):
    LocalShScalarToGridded(parametrisation) {

    std::vector<double> value;
    ASSERT(parametrisation_.userParametrisation().get("grid", value));
    ASSERT(value.size() == 2);
    increments_ = util::Increments(value[0], value[1]);

}


LocalShScalarToRegularLL::~LocalShScalarToRegularLL() {
}


bool LocalShScalarToRegularLL::sameAs(const Action& other) const {
    const LocalShScalarToRegularLL* o = dynamic_cast<const LocalShScalarToRegularLL*>(&other);
    return o && (increments_ == o->increments_);
}


void LocalShScalarToRegularLL::print(std::ostream &out) const {
    out << "LocalShScalarToRegularLL[increments=" << increments_ << "]";
}


const char* LocalShScalarToRegularLL::name() const {
    return "LocalShScalarToRegularLL";
}


const repres::Representation *LocalShScalarToRegularLL::outputRepresentation() const {

    if (!increments_.isPeriodic()) {
        throw eckit::UserError("Spectral transforms only support periodic regular grids", Here());
    }

    // use (non-shifted) global bounding box
    util::BoundingBox bbox;
    increments_.globaliseBoundingBox(bbox, false, false);

    return new repres::latlon::RegularLL(bbox, increments_);
}


void LocalShScalarToRegularLL::setTransOptions(atlas::util::Config& options) const {
    options.set(atlas::option::type("local"));
}


namespace {
static ActionBuilder< LocalShScalarToRegularLL > __action("transform.local-sh-scalar-to-regular-ll");
}


}  // namespace transform
}  // namespace action
}  // namespace mir

