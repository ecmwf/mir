/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/action/transform/LocalShToGridded.h"

#include <algorithm>
#include <iostream>
#include <vector>

#include "eckit/exception/Exceptions.h"
#include "eckit/log/Timer.h"

#include "mir/action/context/Context.h"
#include "mir/action/plan/Action.h"
#include "mir/action/transform/TransCache.h"
#include "mir/caching/InMemoryCache.h"
#include "mir/caching/legendre/LegendreLoader.h"
#include "mir/caching/LegendreCache.h"
#include "mir/config/LibMir.h"
#include "mir/data/MIRField.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/util/MIRStatistics.h"



namespace mir {
namespace action {
namespace transform {


void LocalShToGridded::transform(data::MIRField& field, const repres::Representation& representation, context::Context& ctx) const {

    // Set Trans options, overridden by the user
    // TODO: MIR-183 let Trans decide the best method
    atlas::util::Config options;
    setTransOptions(options);

    bool user_flt = false;
    parametrisation_.userParametrisation().get("atlas-trans-flt", user_flt);
    options.set("flt", user_flt);


    atlas::Grid grid = representation.atlasGrid();
    int truncation = int(field.representation()->truncation());

    atlas::trans::Trans trans(grid, truncation, options);

    {
        eckit::AutoTiming timing(ctx.statistics().timer_, ctx.statistics().sh2gridTiming_);
        sh2grid(field, trans, grid);
    }
}


LocalShToGridded::LocalShToGridded(const param::MIRParametrisation& parametrisation):
    Action(parametrisation) {
}


LocalShToGridded::~LocalShToGridded() {
}


void LocalShToGridded::execute(context::Context& ctx) const {

    repres::RepresentationHandle out(outputRepresentation());

    transform(ctx.field(), *out, ctx);

    ctx.field().representation(out);
}


}  // namespace transform
}  // namespace action
}  // namespace mir

