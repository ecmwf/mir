/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/action/compare/CompareStatistics.h"

#include <limits>
#include <sstream>
#include "eckit/exception/Exceptions.h"
#include "mir/util/Statistics.h"


namespace mir_cmp {


Compare::CompareResults CompareStatistics::getFieldStatisticsScalar(const data::MIRField& field) const {
    typedef util::statistics::ScalarStatistics<double> Calculator;
    Calculator calc(field.hasMissing()? field.missingValue() : std::numeric_limits<double>::quiet_NaN());

    ASSERT(field.dimensions()==1);
    const std::vector<double>& values = field.values(0);

    for (std::vector<double>::const_iterator v=values.begin(); v!=values.end(); ++v) {
        calc(*v);
    }

    Compare::CompareOptions results;
    results.set("min",           calc.min());
    results.set("max",           calc.max());
    results.set("min_index",     calc.minIndex());
    results.set("max_index",     calc.maxIndex());
    results.set("stat_avg",      calc.mean());
    results.set("stat_sd",       calc.standardDeviation());
    results.set("stat_skew",     calc.skewness());
    results.set("stat_kurt",     calc.kurtosis());
    results.set("norm_L1",       calc.normL1());
    results.set("norm_L2",       calc.normL2());
    results.set("norm_Li",       calc.normLinfinity());
    results.set("count",         calc.count());
    results.set("count_missing", calc.countMissing());
    results.set("count_total",   calc.countTotal());
    return results;
}


Compare::CompareResults CompareStatistics::getFieldStatisticsAngleDegrees(const data::MIRField& field) const {
    typedef util::statistics::AngleStatistics<double> Calculator;
    Calculator calc(field.hasMissing()? field.missingValue() : std::numeric_limits<double>::quiet_NaN());

    ASSERT(field.dimensions()==1);
    const std::vector<double>& values = field.values(0);

    for (std::vector<double>::const_iterator v=values.begin(); v!=values.end(); ++v) {
        calc(*v);
    }

    Compare::CompareOptions results;
    std::string mean;
    std::string standardDeviation;
    { std::ostringstream oss; oss << calc.mean()               << "°"; standardDeviation = oss.str(); };
    { std::ostringstream oss; oss << calc.standardDeviation()  << "°"; standardDeviation = oss.str(); };

    results.set("stat_avg",      mean);
    results.set("stat_sd",       standardDeviation);
    results.set("count",         calc.count());
    results.set("count_missing", calc.countMissing());
    results.set("count_total",   calc.countTotal());
    return results;
}


bool CompareStatistics::compare(
        const data::MIRField& field1, const param::MIRParametrisation&,
        const data::MIRField& field2, const param::MIRParametrisation& ) const {

    bool compareAngles =
               options_.get<bool>(".compare-angle-in-polar")
            && field_is_angle_degrees(field1)
            && field_is_angle_degrees(field2);

    return compareResults(
                compareAngles? getFieldStatisticsAngleDegrees(field1) : getFieldStatisticsScalar(field1),
                compareAngles? getFieldStatisticsAngleDegrees(field2) : getFieldStatisticsScalar(field2),
                options_ );
}


namespace {
static ComparisonBuilder<CompareStatistics> compare("statistics");
}


}  // namespace mir_cmp

