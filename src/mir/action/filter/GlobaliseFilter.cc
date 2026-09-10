// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/action/filter/GlobaliseFilter.h"

#include <ostream>

#include "mir/action/context/Context.h"
#include "mir/data/MIRField.h"
#include "mir/repres/Representation.h"
#include "mir/util/Log.h"
#include "mir/util/MIRStatistics.h"


namespace mir::action {


bool GlobaliseFilter::sameAs(const Action& other) const {
    const auto* o = dynamic_cast<const GlobaliseFilter*>(&other);
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

    const auto* out = in->globalise(field);
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


static const ActionBuilder<GlobaliseFilter> __action("filter.globalise");


}  // namespace mir::action
