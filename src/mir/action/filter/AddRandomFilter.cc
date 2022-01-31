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


#include "mir/action/filter/AddRandomFilter.h"

#include <limits>

#include "mir/action/context/Context.h"
#include "mir/data/MIRField.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/stats/Distribution.h"
#include "mir/util/Exceptions.h"


namespace mir {
namespace action {
namespace filter {


AddRandomFilter::AddRandomFilter(const param::MIRParametrisation& param) : Action(param) {
    std::string addRandom;
    param.get("add-random", addRandom);

    distribution_.reset(stats::DistributionFactory::build(addRandom));
    ASSERT(distribution_);
}


bool AddRandomFilter::sameAs(const Action&) const {
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


}  // namespace filter
}  // namespace action
}  // namespace mir
