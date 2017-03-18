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

#include "mir/action/filter/GlobalFilter.h"

#include <iostream>

#include "eckit/exception/Exceptions.h"
#include "mir/action/context/Context.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/util/MIRStatistics.h"
#include "mir/data/MIRField.h"

namespace mir {
namespace action {


GlobalFilter::GlobalFilter(const param::MIRParametrisation &parametrisation):
    Action(parametrisation) {
    // ASSERT(parametrisation.get("user.global", size_));
}


GlobalFilter::~GlobalFilter() {
}


bool GlobalFilter::sameAs(const Action& other) const {
    const GlobalFilter* o = dynamic_cast<const GlobalFilter*>(&other);
    return o;
}

void GlobalFilter::print(std::ostream &out) const {
    out << "GlobalFilter[]";
}


void GlobalFilter::execute(context::Context & ctx) const {

    eckit::AutoTiming timing(ctx.statistics().timer_, ctx.statistics().globalTiming_);
    data::MIRField& field = ctx.field();

    repres::RepresentationHandle in(field.representation());
    repres::Representation* out = in->globalise(field);
    if(out) {
        field.representation(out);
    }
    else {
        eckit::Log::warning() << "Globalise has no effect" << std::endl;
    }
}


namespace {
static ActionBuilder< GlobalFilter > bitmapFilter("filter.globalise");
}


}  // namespace action
}  // namespace mir

