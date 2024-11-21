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


#include "mir/method/nonlinear/MissingIfHeaviestMissing.h"

#include <ostream>
#include <sstream>

#include "eckit/log/JSON.h"
#include "eckit/types/FloatCompare.h"
#include "eckit/utils/MD5.h"

#include "mir/util/Exceptions.h"


namespace mir::method::nonlinear {


MissingIfHeaviestMissing::MissingIfHeaviestMissing(const param::MIRParametrisation& param) : NonLinear(param) {}


bool MissingIfHeaviestMissing::treatment(DenseMatrix& /*A*/, WeightMatrix& W, DenseMatrix& /*B*/,
                                         const MIRValuesVector& values, const double& missingValue) const {

    // correct matrix weigths for the missing values

    ASSERT(W.cols() == values.size());
    auto* outer = W.outer();
    auto* inner = W.inner();
    auto* data = const_cast<WeightMatrix::Scalar*>(W.data());
    bool modif = false;

    #pragma omp parallel for reduction(||:modif) 
    for (WeightMatrix::Size r = 0; r < W.rows(); ++r) {
        WeightMatrix::Size row_start = outer[r];
        WeightMatrix::Size row_end = outer[r + 1];  // Marks the end of this row

        // Initialize variables for tracking missing values and weights in the row
        size_t i_missing         = row_start;
        size_t N_missing         = 0;
        size_t N_entries         = row_end - row_start;
        double sum               = 0.;
        double heaviest          = -1.;
        bool heaviest_is_missing = false;

        for (WeightMatrix::Size i = row_start; i < row_end; ++i) {
            const bool miss = values[inner[i]] == missingValue;

            if (miss) {
                ++N_missing;
                i_missing = i;
            } else {
                sum += data[i];
            }

            if (heaviest < data[i]) {
                heaviest            = data[i];
                heaviest_is_missing = miss;
            }
        }

        if (N_missing > 0) {
            if (N_missing == N_entries || heaviest_is_missing || eckit::types::is_approximately_equal(sum, 0.)) {

                for (WeightMatrix::Size i = row_start; i < row_end; ++i) {
                    data[i] = (i == i_missing) ? 1. : 0.;
                }
            } else {

                const double factor = 1. / sum;
                for (WeightMatrix::Size i = row_start; i < row_end; ++i) {
                    const bool miss = values[inner[i]] == missingValue;
                    data[i] = miss ? 0. : (factor * data[i]);
                }
            }
            modif = true;
        }
    }

    return modif;
}


bool MissingIfHeaviestMissing::sameAs(const NonLinear& other) const {
    const auto* o = dynamic_cast<const MissingIfHeaviestMissing*>(&other);
    return (o != nullptr);
}


void MissingIfHeaviestMissing::print(std::ostream& out) const {
    out << "MissingIfHeaviestMissing[]";
}


void MissingIfHeaviestMissing::hash(eckit::MD5& h) const {
    std::ostringstream s;
    s << *this;
    h.add(s.str());
}


void MissingIfHeaviestMissing::json(eckit::JSON& j) const {
    j.startObject();
    j << "type"
      << "missing-if-heaviest-missing";
    j.endObject();
}


static const NonLinearBuilder<MissingIfHeaviestMissing> __nonlinear("missing-if-heaviest-missing");


}  // namespace mir::method::nonlinear
