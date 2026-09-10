// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/action/filter/AddRandomFilter.h"

#include <limits>

#include "mir/action/context/Context.h"
#include "mir/data/MIRField.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/stats/Distribution.h"
#include "mir/util/Exceptions.h"


namespace mir::action::filter {


AddRandomFilter::AddRandomFilter(const param::MIRParametrisation& param) : Action(param) {
    std::string addRandom;
    param.get("add-random", addRandom);

    distribution_.reset(stats::DistributionFactory::build(addRandom));
    ASSERT(distribution_);
}


bool AddRandomFilter::sameAs(const Action& /*unused*/) const {
    return false;
}


const char* AddRandomFilter::name() const {
    return "AddRandomFilter";
}


void AddRandomFilter::print(std::ostream& out) const {
    out << "AddRandomFilter[distribution=" << *distribution_ << "]";
}


void AddRandomFilter::execute(context::Context& ctx) const {
    auto& field = ctx.field();
    double mv   = field.hasMissing() ? field.missingValue() : std::numeric_limits<double>::quiet_NaN();

    for (size_t i = 0; i < field.dimensions(); ++i) {
        for (double& v : field.direct(i)) {
            if (v != mv) {
                v += (*distribution_)();
            }
        }
    }
}


static const ActionBuilder<AddRandomFilter> __action("filter.add-random");


}  // namespace mir::action::filter
