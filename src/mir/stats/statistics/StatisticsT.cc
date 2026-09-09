// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/stats/statistics/StatisticsT.h"

#include "mir/stats/detail/AngleT.h"
#include "mir/stats/detail/CentralMomentsT.h"
#include "mir/stats/detail/PNormsT.h"
#include "mir/stats/detail/ScalarT.h"
#include "mir/util/Exceptions.h"


namespace mir::stats::statistics {


struct MinMax {};


template <>
void StatisticsT<MinMax>::execute(const data::MIRField& field) {
    Counter::reset(field);

    ASSERT(field.dimensions() == 1);
    for (const auto& value : field.values(0)) {
        count(value);
    }
}


template <>
void StatisticsT<MinMax>::print(std::ostream& out) const {
    out << "Statistics[";
    Counter::print(out);
    out << "]";
}


static const StatisticsBuilder<
    StatisticsT<detail::AngleT<double, detail::AngleScale::DEGREE, detail::AngleSpace::ASYMMETRIC>>>
    __stats1("angle-degree-asymmetric");
static const StatisticsBuilder<
    StatisticsT<detail::AngleT<double, detail::AngleScale::DEGREE, detail::AngleSpace::SYMMETRIC>>>
    __stats2("angle-degree-symmetric");
static const StatisticsBuilder<
    StatisticsT<detail::AngleT<double, detail::AngleScale::RADIAN, detail::AngleSpace::ASYMMETRIC>>>
    __stats3("angle-radian-asymmetric");
static const StatisticsBuilder<
    StatisticsT<detail::AngleT<double, detail::AngleScale::RADIAN, detail::AngleSpace::SYMMETRIC>>>
    __stats4("angle-radian-symmetric");
static const StatisticsBuilder<StatisticsT<detail::CentralMomentsT<double>>> __stats5("central-moments");
static const StatisticsBuilder<StatisticsT<detail::PNormsT<double>>> __stats6("p-norms");
static const StatisticsBuilder<StatisticsT<detail::ScalarT<double>>> __stats7("scalar");
static const StatisticsBuilder<StatisticsT<MinMax>> __stats8("min-max");


}  // namespace mir::stats::statistics
