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
#include "eckit/io/StdFile.h"
#include "eckit/parser/Tokenizer.h"
#include "eckit/utils/Translator.h"

#include "soyuz/data/MIRField.h"
#include "soyuz/param/MIRParametrisation.h"
#include "soyuz/repres/Representation.h"

#include "soyuz/action/FrameFilter.h"


namespace mir {
namespace action {


FrameFilter::FrameFilter(const param::MIRParametrisation &parametrisation):
    Action(parametrisation),
    size_(0) {

    std::string size;
    ASSERT(parametrisation.get("frame", size));
    size_ = eckit::Translator<std::string, long>()(size);
}


FrameFilter::~FrameFilter() {
}


void FrameFilter::print(std::ostream &out) const {
    out << "FrameFilter[size=" << size_ << "]";
}


void FrameFilter::execute(data::MIRField &field) const {

    double missingValue = field.missingValue();
    std::vector<double> &values = field.values();

    const repres::Representation *representation = field.representation();
    size_t count = representation->frame(values, size_, missingValue);

    if (count) {
        field.hasMissing(true);
    } else {
        eckit::Log::warning() << "Frame " << size_ << " has no effect" << std::endl;
    }

}


namespace {
static ActionBuilder< FrameFilter > bitmapFilter("filter.frame");
}


}  // namespace action
}  // namespace mir

