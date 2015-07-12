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

#include "mir/action/misc/AreaCropper.h"

#include <iostream>

#include "eckit/exception/Exceptions.h"
#include "eckit/memory/ScopedPtr.h"

#include "mir/data/MIRField.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/util/BoundingBox.h"

namespace mir {
namespace action {


AreaCropper::AreaCropper(const param::MIRParametrisation &parametrisation):
    Action(parametrisation),
    bbox_() {

    std::vector<double> value;
    ASSERT(parametrisation.get("user.area", value));
    ASSERT(value.size() == 4);

    bbox_ = util::BoundingBox(value[0], value[1], value[2], value[3]);
}


AreaCropper::AreaCropper(const param::MIRParametrisation &parametrisation, const util::BoundingBox &bbox):
    Action(parametrisation),
    bbox_(bbox) {
}

AreaCropper::~AreaCropper() {
}


void AreaCropper::print(std::ostream &out) const {
    out << "AreaCropper[bbox=" << bbox_ << "]";
}


void AreaCropper::execute(data::MIRField &field) const {

    // Keep a pointer on the original representation, as the one in the field will
    // be changed in the loop
    repres::RepresentationHandle representation(field.representation());

    for (size_t i = 0; i < field.dimensions(); i++) {
        const std::vector<double> &values = field.values(i);
        std::vector<double> result;

        const repres::Representation *cropped = representation->crop(bbox_, values, result);

        if (cropped) { // NULL if nothing happend
            field.representation(cropped);
            field.values(result, i);
        }
    }
}


namespace {
static ActionBuilder< AreaCropper > subAreaCropper("crop.area");
}


}  // namespace action
}  // namespace mir

