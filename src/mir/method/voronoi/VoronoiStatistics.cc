// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/method/voronoi/VoronoiStatistics.h"

#include "eckit/log/JSON.h"

#include "mir/method/solver/Statistics.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/stats/Field.h"


namespace mir::method::voronoi {


VoronoiStatistics::VoronoiStatistics(const param::MIRParametrisation& param) :
    VoronoiMethod(param), interpolationStatistics_("maximum") {
    param.get("interpolation-statistics", interpolationStatistics_);

    setSolver(new solver::Statistics(param, stats::FieldFactory::build(interpolationStatistics_, param)));
}


const char* VoronoiStatistics::type() const {
    return "voronoi-statistics";
}


void VoronoiStatistics::json(eckit::JSON& j) const {
    VoronoiMethod::json(j);
    j << "interpolation-statistics" << interpolationStatistics_;
}


static const MethodBuilder<VoronoiStatistics> __builder("voronoi-statistics");


}  // namespace mir::method::voronoi
