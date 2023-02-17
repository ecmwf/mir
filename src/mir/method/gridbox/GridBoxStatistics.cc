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


#include "mir/method/gridbox/GridBoxStatistics.h"

#include "mir/method/solver/Statistics.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/stats/Field.h"


namespace mir::method::gridbox {


GridBoxStatistics::GridBoxStatistics(const param::MIRParametrisation& param) : GridBoxMethod(param) {
    std::string stats = "maximum";
    param.get("interpolation-statistics", stats);

    setSolver(new solver::Statistics(param, stats::FieldFactory::build(stats, param)));
}


static const MethodBuilder<GridBoxStatistics> __builder("grid-box-statistics");


}  // namespace mir::method::gridbox
