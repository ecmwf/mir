/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/action/filter/FrameFilter.h"

#include <iostream>

#include "eckit/exception/Exceptions.h"
#include "mir/action/context/Context.h"
#include "mir/api/MIREstimation.h"
#include "mir/data/MIRField.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/util/MIRStatistics.h"

namespace mir {
namespace action {


FrameFilter::FrameFilter(const param::MIRParametrisation& parametrisation) : Action(parametrisation), size_(0) {
    ASSERT(parametrisation.userParametrisation().get("frame", size_));
}


FrameFilter::~FrameFilter() = default;


bool FrameFilter::sameAs(const Action& other) const {
    auto o = dynamic_cast<const FrameFilter*>(&other);
    return o && (size_ == o->size_);
}

void FrameFilter::print(std::ostream& out) const {
    out << "FrameFilter[size=" << size_ << "]";
}


void FrameFilter::execute(context::Context& ctx) const {

    eckit::AutoTiming timing(ctx.statistics().timer_, ctx.statistics().frameTiming_);
    data::MIRField& field = ctx.field();

    double missingValue = field.missingValue();

    for (size_t i = 0; i < field.dimensions(); i++) {

        MIRValuesVector& values = field.direct(i);

        const repres::Representation* representation = field.representation();
        size_t count                                 = representation->frame(values, size_, missingValue);

        if (count) {
            field.hasMissing(true);
        }
        else {
            eckit::Log::warning() << "Frame " << size_ << " has no effect" << std::endl;
        }
    }
}

const char* FrameFilter::name() const {
    return "FrameFilter";
}

void FrameFilter::estimate(context::Context& ctx, api::MIREstimation& estimation) const {
    const data::MIRField& field = ctx.field();
    ASSERT(field.dimensions() == 1);

    MIRValuesVector dummy;

    size_t count = field.representation()->frame(dummy, size_, 0, true);

    estimation.missingValues(count);
}


namespace {
static ActionBuilder<FrameFilter> frameFilter("filter.frame");
}


}  // namespace action
}  // namespace mir
