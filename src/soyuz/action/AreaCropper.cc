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


#include <iostream>

#include "eckit/exception/Exceptions.h"
#include "eckit/parser/Tokenizer.h"
#include "eckit/utils/Translator.h"

#include "soyuz/data/MIRField.h"
#include "soyuz/param/MIRParametrisation.h"
#include "soyuz/repres/Representation.h"
#include "soyuz/util/BoundingBox.h"

#include "soyuz/action/AreaCropper.h"


namespace mir {
namespace action {


AreaCropper::AreaCropper(const param::MIRParametrisation &parametrisation):
    Action(parametrisation),
    bbox_() {
    std::string value;
    ASSERT(parametrisation.get("user.area", value));

    std::vector<std::string> result;
    eckit::Tokenizer parse("/");

    parse(value, result);
    ASSERT(result.size() == 4);

    bbox_ = util::BoundingBox(
                eckit::Translator<std::string, double>()(result[0]),
                eckit::Translator<std::string, double>()(result[1]),
                eckit::Translator<std::string, double>()(result[2]),
                eckit::Translator<std::string, double>()(result[3])
            );
}


AreaCropper::~AreaCropper() {
}


void AreaCropper::print(std::ostream &out) const {
    out << "AreaCropper[bbox=" << bbox_ << "]";
}


void AreaCropper::execute(data::MIRField &field) const {
    const std::vector<double> &values = field.values();
    std::vector<double> result;

    const repres::Representation *representation = field.representation();
    repres::Representation *cropped = representation->crop(bbox_, values, result);

    if (cropped) { // NULL if nothing happend
        field.representation(cropped);
        field.values(result);
    }
}


namespace {
static ActionBuilder< AreaCropper > subAreaCropper("crop.area");
}


}  // namespace action
}  // namespace mir

