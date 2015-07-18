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

#include "mir/action/misc/ReferencePattern.h"

#include <iostream>

#include "eckit/exception/Exceptions.h"
#include "eckit/memory/ScopedPtr.h"

#include "mir/data/MIRField.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/util/BoundingBox.h"

namespace mir {
namespace action {


ReferencePattern::ReferencePattern(const param::MIRParametrisation &parametrisation):
    Action(parametrisation) {

    // std::vector<double> value;
    // ASSERT(parametrisation.get("user.area", value));
    // ASSERT(value.size() == 4);

    // bbox_ = util::BoundingBox(value[0], value[1], value[2], value[3]);
}


ReferencePattern::~ReferencePattern() {
}


void ReferencePattern::print(std::ostream &out) const {
    out << "ReferencePattern["  << "]";
}


void ReferencePattern::execute(data::MIRField &field) const {

    repres::RepresentationHandle representation(field.representation());

    for (size_t i = 0; i < field.dimensions(); i++) {
        std::vector<double> &values = field.values(i);
        representation->pattern(values, field.hasMissing(), field.missingValue());
    }
}


namespace {
static ActionBuilder< ReferencePattern > action("misc.pattern");
}


}  // namespace action
}  // namespace mir

