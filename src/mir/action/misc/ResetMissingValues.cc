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

#include <algorithm>
#include <memory>
#include <ostream>

#include "mir/action/context/Context.h"
#include "mir/data/MIRField.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/Trace.h"


namespace mir {
namespace action {


void ResetMissingValues::execute(context::Context& ctx) const {
    trace::Timer timer("ResetMissingValues");

    auto& field = ctx.field();

    repres::RepresentationHandle rep(field.representation());
    auto hasMissing   = field.hasMissing();
    auto missingValue = field.missingValue();

    for (size_t d = 0; d < field.dimensions(); d++) {
        const auto& values = field.values(d);
        ASSERT(!values.empty());

        if (!hasMissing) {
            missingValue = values.front();
            Log::debug() << "ResetMissingValues: introducing missing values (missingValue=" << missingValue << ")"
                         << std::endl;
        }

        MIRValuesVector result(values.size(), missingValue);

        for (const std::unique_ptr<repres::Iterator> it(rep->iterator()); it->next();) {
            auto i    = it->index();
            result[i] = values.at(i);
        }

        auto count = std::count(result.begin(), result.end(), missingValue);
        if (count != 0) {
            Log::info() << "ResetMissingValues: " << Log::Pretty(count, {"missing value"}) << " of "
                        << Log::Pretty(values.size(), {"total value"}) << std::endl;

            field.hasMissing(true);
            field.missingValue(missingValue);
            field.update(result, d);
        }
        else {
            field.hasMissing(false);
        }
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
