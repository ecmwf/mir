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


#include "mir/action/statistics/PNorms.h"

#include <cmath>
#include <sstream>
//#include <map>
//#include <ostream>
//#include "eckit/exception/Exceptions.h"
//#include "eckit/log/Log.h"
//#include "eckit/log/Plural.h"
//#include "mir/action/context/Context.h"
//#include "mir/config/LibMir.h"
//#include "mir/input/MIRInput.h"
#include "mir/util/Compare.h"


namespace mir {
namespace action {
namespace statistics {


PNorms::PNorms(const param::MIRParametrisation& parametrisation) :
    Statistics(parametrisation) {
}


void PNorms::operator+=(const PNorms&) {
#if 0
    normL1_       += other.normL1_;
    sumSquares_   += other.sumSquares_;
    normLinfinity_ = std::max(normLinfinity_, other.normLinfinity_);
    return true;
#endif
}


bool PNorms::sameAs(const action::Action& other) const {
    const PNorms* o = dynamic_cast<const PNorms*>(&other);
    return o; //(o && options_ == o->options_);
}


void PNorms::calculate(const data::MIRField& field, Results& results) const {

    results.reset();
    util::compare::IsMissingFn isMissing( field.hasMissing()?
                                              field.missingValue() :
                                              std::numeric_limits<double>::quiet_NaN() );

    for (size_t w = 0; w < field.dimensions(); ++w) {
        double normL1 = 0;
        double normLi = 0;
        double sumSq = 0;
        size_t missing = 0;

        const std::vector<double>& values = field.values(w);
        for (size_t j = 0; j < values.size(); ++j) {
            if (isMissing(values[j])) {
                ++missing;
                continue;
            }

            double v = values[j];
            normL1 += std::abs(v);
            normLi  = std::max(normLi , std::abs(v));
            sumSq  += v*v;
        }
        double normL2 = std::sqrt(sumSq);

        std::string head;
        if (field.dimensions()>1) {
            std::ostringstream s;
            s << '#' << (w+1) << ' ';
            head = s.str();
        }

        results.set(head + " L1",      normL1);
        results.set(head + " L2",      normL2);
        results.set(head + " Li",      normLi);
        results.set(head + " missing", missing);
    }
}


namespace {
// Name capitalized according to: https://en.wikipedia.org/wiki/Lp_space
static StatisticsBuilder<PNorms> statistics("p-norms");
}


}  // namespace statistics
}  // namespace action
}  // namespace mir

