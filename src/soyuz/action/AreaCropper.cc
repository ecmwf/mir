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

#include "soyuz/action/AreaCropper.h"


namespace mir {
namespace action {


AreaCropper::AreaCropper(const MIRParametrisation& parametrisation):
    Action(parametrisation),
    north_(0),
    west_(0),
    south_(0),
    east_(0) {
    std::string value;
    ASSERT(parametrisation.get("user.area", value));

    std::vector<std::string> result;
    eckit::Tokenizer parse("/");

    parse(value, result);
    ASSERT(result.size() == 4);

    north_ = eckit::Translator<std::string, double>()(result[0]);
    west_ = eckit::Translator<std::string, double>()(result[1]);
    south_ = eckit::Translator<std::string, double>()(result[2]);
    east_ = eckit::Translator<std::string, double>()(result[3]);
}


AreaCropper::~AreaCropper() {
}


void AreaCropper::print(std::ostream& out) const {
    out << "AreaCropper[";
    out << "north=" << north_;
    out << ",west=" << west_;
    out << ",south=" << south_;
    out << ",east=" << east_;
    out << "]";
}


void AreaCropper::execute(data::MIRField& field) const {
    const std::vector<double> &values = field.values();
    std::vector<double> result;

    const repres::Representation* representation = field.representation();
    repres::Representation* cropped = representation->crop(north_, west_, south_, east_, values, result);

    if(cropped) { // NULL if nothing happend
        field.representation(cropped);
        field.values(result);
    }
}


namespace {
static ActionBuilder< AreaCropper > subAreaCropper("crop.area");
}


}  // namespace action
}  // namespace mir

