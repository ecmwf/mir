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

#include "mir/action/filter/SubsetFilter.h"

#include <iostream>

#include "eckit/exception/Exceptions.h"
#include "mir/action/context/Context.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/util/MIRStatistics.h"
#include "mir/data/MIRField.h"

namespace mir {
namespace action {


SubsetFilter::SubsetFilter(const param::MIRParametrisation &parametrisation):
    Action(parametrisation) {

    std::vector<double> value;

    ASSERT(parametrisation.get("user.subset", value));
    ASSERT(value.size() == 2);

    increments_ = util::Increments(value[0], value[1]);
}


SubsetFilter::~SubsetFilter() {
}


bool SubsetFilter::sameAs(const Action& other) const {
    const SubsetFilter* o = dynamic_cast<const SubsetFilter*>(&other);
    return o && (increments_ == o->increments_);
}

void SubsetFilter::print(std::ostream &out) const {
    out << "SubsetFilter[increments=" << increments_ << "]";
}


void SubsetFilter::execute(context::Context & ctx) const {


    eckit::AutoTiming timing(ctx.statistics().timer_, ctx.statistics().subsetTiming_);
    data::MIRField& field = ctx.field();

    repres::RepresentationHandle in(field.representation());
    repres::Representation* out = in->subset(field, increments_);
    ASSERT(out);

    field.representation(out);

}


namespace {
static ActionBuilder< SubsetFilter > bitmapFilter("filter.subset");
}


}  // namespace action
}  // namespace mir

