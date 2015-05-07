/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date Apr 2015

#include "mir/output/UVOutput.h"

#include <iostream>

#include "eckit/exception/Exceptions.h"
#include "mir/data/MIRField.h"
#include "mir/param/RuntimeParametrisation.h"
#include "mir/repres/Representation.h"


namespace mir {
namespace output {


UVOutput::UVOutput(MIROutput &u_component, MIROutput &v_component):
    u_component_(u_component),
    v_component_(v_component) {
}


UVOutput::~UVOutput() {
}

void UVOutput::print(std::ostream &out) const {
    out << "UVOutput[u_component=" << u_component_ << ", v_component=" << v_component_ << "]";
}

void UVOutput::copy(const param::MIRParametrisation &, input::MIRInput &input) {
    NOTIMP;
}

void UVOutput::save(const param::MIRParametrisation &param, input::MIRInput &input, data::MIRField &field) {
    ASSERT(field.dimensions() == 2);

    data::MIRField u(field.representation()->clone(), field.hasMissing(), field.missingValue());
    u.values(field.values(0), 0);

    data::MIRField v(field.representation()->clone(), field.hasMissing(), field.missingValue());
    v.values(field.values(1), 0);

    param::RuntimeParametrisation u_runtime(param);
    u_runtime.set("u-component", true);
    u_component_.save(u_runtime, input, u);

    param::RuntimeParametrisation v_runtime(param);
    v_runtime.set("v-component", true);
    v_component_.save(v_runtime, input, v);
}

}  // namespace output
}  // namespace mir

