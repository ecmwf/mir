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

#include "mir/output/VectorOutput.h"

#include <iostream>

#include "eckit/exception/Exceptions.h"
#include "mir/input/VectorInput.h"
#include "eckit/exception/Exceptions.h"
#include "mir/data/MIRField.h"
#include "mir/param/RuntimeParametrisation.h"
#include "mir/repres/Representation.h"


namespace mir {
namespace output {


VectorOutput::VectorOutput(MIROutput &component1, MIROutput &v_component):
    component1_(component1),
    component2_(v_component) {
}


VectorOutput::~VectorOutput() {
}


void VectorOutput::copy(const param::MIRParametrisation &param, input::MIRInput &input) {
    try {
        input::VectorInput& v = dynamic_cast<input::VectorInput&>(input);
        component1_.copy(param, v.component1_);
        component2_.copy(param, v.component2_);

    } catch (std::bad_cast &) {
        eckit::StrStream os;
        os << "VectorOutput::copy() not implemented for input of type: " << input << eckit::StrStream::ends;
        throw eckit::SeriousBug(std::string(os));
    }
}

void VectorOutput::save(const param::MIRParametrisation &param, input::MIRInput &input, data::MIRField &field) {

    ASSERT(field.dimensions() == 2);

    data::MIRField u(field.representation()->clone(), field.hasMissing(), field.missingValue());
    u.values(field.values(0), 0);

    data::MIRField v(field.representation()->clone(), field.hasMissing(), field.missingValue());
    v.values(field.values(1), 0);

    param::RuntimeParametrisation u_runtime(param);
    u_runtime.set("param-id", component1ParamId(input));
    component1_.save(u_runtime, input, u);

    param::RuntimeParametrisation v_runtime(param);
    v_runtime.set("param-id", component2ParamId(input)); // TODO: Find something better
    component2_.save(v_runtime, input, v);
}


// Default is same as input
// TODO: Something more elegant

long VectorOutput::component1ParamId(input::MIRInput &input) const  {
    try {
        input::VectorInput& v = dynamic_cast<input::VectorInput&>(input);
        const param::MIRParametrisation& metadata = v.component1_.parametrisation();
        long paramId;
        ASSERT(metadata.get("paramId", paramId));
        return paramId;
    } catch (std::bad_cast &) {
        eckit::StrStream os;
        os << "VectorOutput::component1ParamId() not implemented for input of type: " << input << eckit::StrStream::ends;
        throw eckit::SeriousBug(std::string(os));
    }
}

long VectorOutput::component2ParamId(input::MIRInput &input) const {
    try {
        input::VectorInput& v = dynamic_cast<input::VectorInput&>(input);
        const param::MIRParametrisation& metadata = v.component2_.parametrisation();
        long paramId;
        ASSERT(metadata.get("paramId", paramId));
        return paramId;
    } catch (std::bad_cast &) {
        eckit::StrStream os;
        os << "VectorOutput::component2ParamId() not implemented for input of type: " << input << eckit::StrStream::ends;
        throw eckit::SeriousBug(std::string(os));
    }
}

}  // namespace output
}  // namespace mir

