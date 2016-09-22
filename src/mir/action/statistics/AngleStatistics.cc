/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @date Aug 2016


#include "mir/action/statistics/AngleStatistics.h"

#include <sstream>


namespace mir {
namespace action {
namespace statistics {


AngleStatistics::AngleStatistics(const param::MIRParametrisation& parametrisation) :
    Statistics(parametrisation) {
}


void AngleStatistics::operator+=(const AngleStatistics& other) {
    stats_ += other.stats_;
}


bool AngleStatistics::sameAs(const action::Action& other) const {
    const AngleStatistics* o = dynamic_cast<const AngleStatistics*>(&other);
    return o; //(o && options_ == o->options_);
}


void AngleStatistics::calculate(const data::MIRField& field, Results& results) const {
    results.reset();

    NOTIMP;

//     data::FieldInfo info = field.info();
//     bool degrees   = info.isAngleInDegrees();
//     bool symmetric = info.isAngleSymmetric();

//     for (size_t w = 0; w < field.dimensions(); ++w) {
//         const std::vector<double>& values = field.values(w);

//         stats_.reset(
//                     field.hasMissing()? field.missingValue() : std::numeric_limits<double>::quiet_NaN(),
//                     degrees,
//                     symmetric );
//         for (size_t i = 0; i < values.size(); ++ i) {
//             stats_(values[i]);
//         }

//         std::string head;
//         if (field.dimensions()>1) {
//             std::ostringstream s;
//             s << '#' << (w+1) << ' ';
//             head = s.str();
//         }

//         results.set(head + "min",               stats_.min());
//         results.set(head + "max",               stats_.max());
//         results.set(head + "minIndex",          stats_.minIndex());
//         results.set(head + "maxIndex",          stats_.maxIndex());

//         results.set(head + "mean",              stats_.mean());
//         results.set(head + "variance",          stats_.variance());
// //      results.set(head + "skewness",          stats_.skewness());    // works, but is it meaningful for circular quantities?
// //      results.set(head + "kurtosis",          stats_.kurtosis());    // works, but is it meaningful for circular quantities?
//         results.set(head + "standardDeviation", stats_.standardDeviation());

//         results.set(head + "count",   stats_.countNonMissing());
//         results.set(head + "missing", stats_.countMissing());

//     }
}


namespace {
static StatisticsBuilder<AngleStatistics> statistics("AngleStatistics");
}


}  // namespace statistics
}  // namespace action
}  // namespace mir

