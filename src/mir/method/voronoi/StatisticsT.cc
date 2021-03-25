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


#include "mir/method/voronoi/StatisticsT.h"

#include "mir/method/detail/CounterStats.h"
#include "mir/method/detail/GeneralStatsT.h"
#include "mir/method/solver/StatisticsT.h"


namespace mir {
namespace method {
namespace voronoi {


template <typename T>
StatisticsT<T>::StatisticsT(const param::MIRParametrisation& param) : VoronoiMethod(param) {
    setSolver(new solver::StatisticsT<T>(param));
}


static MethodBuilder<StatisticsT<detail::Maximum>> __builder_1("voronoi-maximum");
static MethodBuilder<StatisticsT<detail::Minimum>> __builder_2("voronoi-minimum");
static MethodBuilder<StatisticsT<detail::CountAboveUpperLimit>> __builder_3("voronoi-count-above-upper-limit");
static MethodBuilder<StatisticsT<detail::CountBelowLowerLimit>> __builder_4("voronoi-count-below-lower-limit");
static MethodBuilder<StatisticsT<detail::Count>> __builder_5("voronoi-count");
static MethodBuilder<StatisticsT<detail::Mean>> __builder_6("voronoi-mean");
static MethodBuilder<StatisticsT<detail::Variance>> __builder_7("voronoi-variance");
static MethodBuilder<StatisticsT<detail::Skewness>> __builder_8("voronoi-skewness");
static MethodBuilder<StatisticsT<detail::Kurtosis>> __builder_9("voronoi-kurtosis");
static MethodBuilder<StatisticsT<detail::StandardDeviation>> __builder_10("voronoi-stddev");
static MethodBuilder<StatisticsT<detail::ModeIntegral>> __builder_11("voronoi-mode-integral");
static MethodBuilder<StatisticsT<detail::ModeReal>> __builder_12("voronoi-mode-real");
static MethodBuilder<StatisticsT<detail::ModeBoolean>> __builder_13("voronoi-mode-boolean");


}  // namespace voronoi
}  // namespace method
}  // namespace mir
