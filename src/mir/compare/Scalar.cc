/*
 * (C) Copyright 1996- ECMWF.
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


#include "mir/compare/Scalar.h"

#include <cmath>
#include "mir/config/LibMir.h"
#include "mir/data/MIRField.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/Compare.h"
#include "eckit/exception/Exceptions.h"


namespace mir {
namespace compare {


Scalar::Scalar(const param::MIRParametrisation& param1, const param::MIRParametrisation& param2) :
    Comparator(param1, param2) {
    absoluteError_ = getSameParameter<double>("absolute-error");
    ASSERT(absoluteError_ > 0);
}


Scalar::~Scalar() = default;


void Scalar::execute(const data::MIRField& field1, const data::MIRField& field2) const {
    ASSERT(field1.dimensions() == field2.dimensions());

    for (size_t w = 0; w < field1.dimensions(); ++w) {

        const MIRValuesVector& values1 = field1.values(w);
        const MIRValuesVector& values2 = field2.values(w);
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

                } else if (!isMissing1(values1[i]) && !isMissing1(values2[i])) {

                    // FIXME
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


void Scalar::print(std::ostream& out) const {
    out << "ScalarComparator["
        << "absoluteError=" << absoluteError_
        << "]";
}


namespace {
ComparatorBuilder<Scalar> __scalar("scalar");
}


}  // namespace compare
}  // namespace mir


