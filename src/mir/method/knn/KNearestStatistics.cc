// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/method/knn/KNearestStatistics.h"

#include <ostream>

#include "mir/method/knn/pick/Pick.h"
#include "mir/method/solver/Statistics.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/stats/Field.h"
#include "mir/util/Exceptions.h"


namespace mir::method::knn {


KNearestStatistics::KNearestStatistics(const param::MIRParametrisation& param) :
    KNearestNeighbours(param), distanceWeighting_(param) {
    std::string method = "nclosest-or-nearest";
    param.get("nearest-method", method);
    pick_.reset(pick::PickFactory::build(method, param));

    std::string stats = "maximum";
    param.get("interpolation-statistics", stats);
    setSolver(new solver::Statistics(param, stats::FieldFactory::build(stats, param)));
}


const char* KNearestStatistics::type() const {
    return "k-nearest-statistics";
}


bool KNearestStatistics::sameAs(const Method& other) const {
    const auto* o = dynamic_cast<const KNearestStatistics*>(&other);
    return (o != nullptr) && KNearestStatistics::sameAs(other);
}


const pick::Pick& KNearestStatistics::pick() const {
    ASSERT(pick_);
    return *pick_;
}


const distance::DistanceWeighting& KNearestStatistics::distanceWeighting() const {
    return distanceWeighting_;
}


static const MethodBuilder<KNearestStatistics> __method1("k-nearest-statistics");
static const MethodBuilder<KNearestStatistics> __method2("k-nearest-neighbours-statistics");
static const MethodBuilder<KNearestStatistics> __method3("k-nearest-neighbors-statistics");


}  // namespace mir::method::knn
