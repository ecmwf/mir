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


#include "mir/method/nonlinear/MissingIfAllMissing.h"

#include <ostream>
#include <sstream>

#include "eckit/log/JSON.h"
#include "eckit/types/FloatCompare.h"
#include "eckit/utils/MD5.h"

#include "mir/util/Exceptions.h"
#include "mir/util/Types.h"


namespace mir::method::nonlinear {


MissingIfAllMissing::MissingIfAllMissing(const param::MIRParametrisation& param) : NonLinear(param) {}


bool MissingIfAllMissing::treatment(MethodWeighted::Matrix& /*A*/, MethodWeighted::WeightMatrix& W,
                                    MethodWeighted::Matrix& /*B*/, const MIRValuesVector& values,
                                    const double& missingValue) const {

    // correct matrix weigths for the missing values
    // (force a missing value only if all row values are missing)
    ASSERT(W.cols() == values.size());

    auto* data = const_cast<WeightMatrix::Scalar*>(W.data());
    bool modif = false;

    WeightMatrix::Size i = 0;
    WeightMatrix::iterator it(W);
    for (WeightMatrix::Size r = 0; r < W.rows(); ++r) {
        const WeightMatrix::iterator end = W.end(r);

        // count missing values, accumulate weights (disregarding missing values)
        size_t i_missing = i;
        size_t N_missing = 0;
        size_t N_entries = 0;
        double sum       = 0.;

        WeightMatrix::iterator kt(it);
        WeightMatrix::Size k = i;
        for (; it != end; ++it, ++i, ++N_entries) {

            const bool miss = values[it.col()] == missingValue;

            if (miss) {
                ++N_missing;
                i_missing = i;
            }
            else {
                sum += *it;
            }
        }

        // weights redistribution: zero-weight all missing values, linear re-weighting for the others;
        // the result is missing value if all values in row are missing
        if (N_missing > 0) {
            if (N_missing == N_entries || eckit::types::is_approximately_equal(sum, 0.)) {

                for (WeightMatrix::Size j = k; j < k + N_entries; ++j) {
                    data[j] = j == i_missing ? 1. : 0.;
                }
            }
            else {

                const double factor = 1. / sum;
                for (WeightMatrix::Size j = k; j < k + N_entries; ++j, ++kt) {
                    const bool miss = values[kt.col()] == missingValue;
                    data[j]         = miss ? 0. : (factor * data[j]);
                }
            }
            modif = true;
        }
    }

    return modif;
}


bool MissingIfAllMissing::sameAs(const NonLinear& other) const {
    const auto* o = dynamic_cast<const MissingIfAllMissing*>(&other);
    return (o != nullptr);
}


void MissingIfAllMissing::print(std::ostream& out) const {
    out << "MissingIfAllMissing[]";
}


void MissingIfAllMissing::hash(eckit::MD5& h) const {
    std::ostringstream s;
    s << *this;
    h.add(s.str());
}


void MissingIfAllMissing::json(eckit::JSON& j) const {
    j.startObject();
    j << "type" << "missing-if-all-missing";
    j.endObject();
}


static const NonLinearBuilder<MissingIfAllMissing> __nonlinear("missing-if-all-missing");


}  // namespace mir::method::nonlinear
