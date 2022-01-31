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


#include "mir/method/nonlinear/MissingIfAnyMissing.h"

#include <ostream>
#include <sstream>

#include "eckit/utils/MD5.h"

#include "mir/util/Exceptions.h"


namespace mir {
namespace method {
namespace nonlinear {


MissingIfAnyMissing::MissingIfAnyMissing(const param::MIRParametrisation& param) : NonLinear(param) {}


bool MissingIfAnyMissing::treatment(MethodWeighted::Matrix&, MethodWeighted::WeightMatrix& W, MethodWeighted::Matrix&,
                                    const MIRValuesVector& values, const double& missingValue) const {

    // correct matrix weigths for the missing values
    // (force a missing value only if any row values is missing)
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

        WeightMatrix::iterator kt(it);
        WeightMatrix::Size k = i;
        for (; it != end; ++it, ++i, ++N_entries) {

            const bool miss = values[it.col()] == missingValue;

            if (miss) {
                ++N_missing;
                i_missing = i;
            }
        }

        // if any values in row are missing, force missing value
        if (N_missing > 0) {
            for (WeightMatrix::Size j = k; j < k + N_entries; ++j) {
                data[j] = j == i_missing ? 1. : 0.;
            }
            modif = true;
        }
    }

    return modif;
}


bool MissingIfAnyMissing::sameAs(const NonLinear& other) const {
    const auto* o = dynamic_cast<const MissingIfAnyMissing*>(&other);
    return (o != nullptr);
}


void MissingIfAnyMissing::print(std::ostream& out) const {
    out << "MissingIfAnyMissing[]";
}


void MissingIfAnyMissing::hash(eckit::MD5& h) const {
    std::ostringstream s;
    s << *this;
    h.add(s.str());
}


static const NonLinearBuilder<MissingIfAnyMissing> __nonlinear("missing-if-any-missing");


}  // namespace nonlinear
}  // namespace method
}  // namespace mir
