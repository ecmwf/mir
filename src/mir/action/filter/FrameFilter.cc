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

#include "mir/action/filter/FrameFilter.h"

#include <iostream>

#include "eckit/exception/Exceptions.h"
#include "mir/data/MIRField.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/util/MIRStatistics.h"

namespace mir {
namespace action {


FrameFilter::FrameFilter(const param::MIRParametrisation &parametrisation):
    Action(parametrisation),
    size_(0) {
    ASSERT(parametrisation.get("user.frame", size_));
}


FrameFilter::~FrameFilter() {
}


bool FrameFilter::sameAs(const Action& other) const {
    const FrameFilter* o = dynamic_cast<const FrameFilter*>(&other);
    return o && (size_ == o->size_);
}

void FrameFilter::print(std::ostream &out) const {
    out << "FrameFilter[size=" << size_ << "]";
}


void FrameFilter::execute(data::MIRField & field, util::MIRStatistics& statistics) const {

    eckit::AutoTiming timing(statistics.timer_, statistics.frameTiming_);

    for (size_t i = 0; i < field.dimensions(); i++ ) {

        double missingValue = field.missingValue();
        std::vector<double> &values = field.direct(i);

        const repres::Representation *representation = field.representation();
        size_t count = representation->frame(values, size_, missingValue);

        if (count) {
            field.hasMissing(true);
        } else {
            eckit::Log::warning() << "Frame " << size_ << " has no effect" << std::endl;
        }
    }
}


namespace {
static ActionBuilder< FrameFilter > bitmapFilter("filter.frame");
}


}  // namespace action
}  // namespace mir

