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


#include "mir/action/misc/ResetMissingValues.h"

#include <limits>
#include <ostream>

#include "mir/action/context/Context.h"
#include "mir/data/MIRField.h"
#include "mir/repres/Representation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/Trace.h"


namespace mir {
namespace action {


void ResetMissingValues::execute(context::Context& ctx) const {
    trace::Timer timer("ResetMissingValues");

    auto& field = ctx.field();
    if (field.hasMissing()) {
        return;
    }

    repres::RepresentationHandle rep(field.representation());

    auto missingValue = std::numeric_limits<double>::lowest();

    size_t count = 0;
    size_t total = 0;
    for (size_t d = 0; d < field.dimensions(); d++) {
        auto& values = field.direct(d);
        ASSERT(!values.empty());

        auto oldMissingValue = values.front();
        for (auto& v : values) {
            if (v == oldMissingValue) {
                v = missingValue;
                ++count;
            }
        }

        total += values.size();
    }

    if (count != 0) {
        Log::info() << "ResetMissingValues: introducing missing values (missingValue=" << missingValue << "), "
                    << Log::Pretty(count, {"missing value"}) << " of " << Log::Pretty(total, {"total value"})
                    << std::endl;

        field.hasMissing(true);
        field.missingValue(missingValue);
    }
    else {
        field.hasMissing(false);
    }
}


void ResetMissingValues::print(std::ostream& out) const {
    out << name() << "[]";
}


bool ResetMissingValues::sameAs(const Action& other) const {
    auto o = dynamic_cast<const ResetMissingValues*>(&other);
    return (o != nullptr);
}


const char* ResetMissingValues::name() const {
    return "ResetMissingValues";
}


void ResetMissingValues::estimate(context::Context&, api::MIREstimation&) const {
    // do nothing
}


static ActionBuilder<ResetMissingValues> __action("misc.reset-missing-values");


}  // namespace action
}  // namespace mir
