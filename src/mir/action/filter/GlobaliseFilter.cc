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


#include "mir/action/filter/GlobaliseFilter.h"

#include <ostream>

#include "mir/action/context/Context.h"
#include "mir/data/MIRField.h"
#include "mir/repres/Representation.h"
#include "mir/util/Log.h"
#include "mir/util/MIRStatistics.h"


namespace mir {
namespace action {


GlobaliseFilter::GlobaliseFilter(const param::MIRParametrisation& parametrisation) : Action(parametrisation) {
    // ASSERT(parametrisation.user().get("global", size_));
}


GlobaliseFilter::~GlobaliseFilter() = default;


bool GlobaliseFilter::sameAs(const Action& other) const {
    auto o = dynamic_cast<const GlobaliseFilter*>(&other);
    return (o != nullptr);
}


void GlobaliseFilter::print(std::ostream& out) const {
    out << "GlobaliseFilter[]";
}


bool GlobaliseFilter::deleteWithNext(const Action& next) {
    return next.isCropAction();
}


void GlobaliseFilter::execute(context::Context& ctx) const {
    auto timing(ctx.statistics().globaliseTimer());

    auto& field = ctx.field();
    repres::RepresentationHandle in(field.representation());

    auto out = in->globalise(field);
    if (out == nullptr) {
        Log::warning() << "Globalise has no effect" << std::endl;
    }
    else {
        field.representation(out);
    }
}


const char* GlobaliseFilter::name() const {
    return "GlobaliseFilter";
}


static ActionBuilder<GlobaliseFilter> __action("filter.globalise");


}  // namespace action
}  // namespace mir
