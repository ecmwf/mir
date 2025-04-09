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

#include <ostream>

#include "mir/action/context/Context.h"
#include "mir/api/MIREstimation.h"
#include "mir/data/MIRField.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/MIRStatistics.h"


namespace mir::action {


FrameFilter::FrameFilter(const param::MIRParametrisation& param) : Action(param), size_(0) {
    ASSERT(parametrisation().userParametrisation().get("frame", size_));
}


bool FrameFilter::sameAs(const Action& other) const {
    const auto* o = dynamic_cast<const FrameFilter*>(&other);
    return (o != nullptr) && (size_ == o->size_);
}


void FrameFilter::print(std::ostream& out) const {
    out << "FrameFilter[size=" << size_ << "]";
}


void FrameFilter::execute(context::Context& ctx) const {
    auto timing(ctx.statistics().frameTimer());

    data::MIRField& field = ctx.field();
    double missingValue   = field.missingValue();

    for (size_t i = 0; i < field.dimensions(); i++) {

        MIRValuesVector& values = field.direct(i);

        const auto* representation = field.representation();
        if (representation->frame(values, size_, missingValue) == 0) {
            Log::warning() << "Frame " << size_ << " has no effect" << std::endl;
        }
        else {
            field.hasMissing(true);
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


static const ActionBuilder<FrameFilter> __action("filter.frame");


}  // namespace mir::action
