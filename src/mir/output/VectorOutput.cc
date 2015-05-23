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
#include "mir/data/MIRField.h"
#include "mir/param/RuntimeParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/input/VectorInput.h"


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

    component1_.save(param, input, u);
    component2_.save(param, input, v);
}

}  // namespace output
}  // namespace mir

