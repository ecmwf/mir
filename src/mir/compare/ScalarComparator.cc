/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date Apr 2015

#include <cmath>

#include "eckit/exception/Exceptions.h"
#include "mir/compare/ScalarComparator.h"
#include "mir/config/LibMir.h"
#include "mir/data/MIRField.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/Compare.h"
#include "eckit/exception/Exceptions.h"


namespace mir {
namespace compare {


ScalarComparator::ScalarComparator(const param::MIRParametrisation& param1, const param::MIRParametrisation& param2) {
    double absoluteError1;
    double absoluteError2;
    ASSERT(param1.get("absolute-error", absoluteError1));
    ASSERT(param2.get("absolute-error", absoluteError2));
    ASSERT(absoluteError1 == absoluteError2);

    absoluteError_ = absoluteError1;
    ASSERT(absoluteError_ > 0);
}


ScalarComparator::~ScalarComparator() {
}

void ScalarComparator::execute(const data::MIRField& field1, const data::MIRField& field2) const {
    ASSERT(field1.dimensions() == field2.dimensions());

    for (size_t w = 0; w < field1.dimensions(); ++w) {

        const std::vector<double>& values1 = field1.values(w);
        const std::vector<double>& values2 = field2.values(w);
        ASSERT(values1.size() == values2.size());

        if (field1.hasMissing() || field2.hasMissing()) {
            NOTIMP;

            util::compare::IsMissingFn isMissing1(field1.hasMissing()? field1.missingValue() : std::numeric_limits<double>::quiet_NaN());
            util::compare::IsMissingFn isMissing2(field2.hasMissing()? field2.missingValue() : std::numeric_limits<double>::quiet_NaN());

            for (size_t i = 0; i < values1.size(); ++i) {
                if (isMissing1(values1[i] != isMissing1(values2[i]))) {

                    std::ostringstream oss;
                    oss << "missing value mismatch at point " << (i+1);
                    throw eckit::UserError(oss.str());

                } else if (!isMissing1(values1[i] && !isMissing1(values2[i]))) {


                }
            }

        } else {

            for (size_t i = 0; i < values1.size(); ++i) {
                double diff = std::abs(values1[i] - values2[i]);
                if (diff > absoluteError_) {

                    std::ostringstream oss;
                    oss << "value mismatch at point " << (i+1) << ": V1=" << values1[i] << ", V2=" << values2[i] << ", |V2-V1|=" << diff;
                    throw eckit::UserError(oss.str());

                }
            }

        }

    }
}


void ScalarComparator::print(std::ostream& out) const {
    out << "ScalarComparator["
        << "absoluteError=" << absoluteError_
        << "]";
}


namespace {
ComparatorBuilder<ScalarComparator> __scalarComparator("scalar-comparator");
}


}  // namespace compare
}  // namespace mir


