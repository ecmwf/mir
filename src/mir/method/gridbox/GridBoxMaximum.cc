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


#include "mir/method/gridbox/GridBoxMaximum.h"

#include <limits>
#include <ostream>
#include <sstream>

#include "eckit/utils/MD5.h"

#include "mir/method/nonlinear/NonLinear.h"
#include "mir/util/Exceptions.h"


namespace mir {
namespace method {
namespace gridbox {


struct NonLinearGridBoxMaximum : nonlinear::NonLinear {
    using NonLinear::NonLinear;
    bool treatment(MethodWeighted::Matrix& /*A*/, MethodWeighted::WeightMatrix& W, MethodWeighted::Matrix& /*B*/,
                   const data::MIRValuesVector& values, const double& missingValue) const override {

        // locate rows referring input maximum value, and set rows to pick only those
        ASSERT(W.cols() == values.size());

        std::vector<WeightMatrix::Triplet> triplets;
        triplets.reserve(W.rows());

        WeightMatrix::iterator it(W);
        for (WeightMatrix::Size r = 0, c = 0; r < W.rows(); ++r) {
            double max = std::numeric_limits<double>::lowest();
            bool found = false;

            for (; it != W.end(r); ++it) {
                ASSERT(it.col() < values.size());
                auto value = values[it.col()];
                if (!found || (max < value && value != missingValue)) {
                    max = value;
                    c   = it.col();
                }
                found = true;
            }

            if (found) {
                triplets.emplace_back(WeightMatrix::Triplet(r, c, 1.));
            }
        }

        ASSERT(!triplets.empty());
        WeightMatrix N(W.rows(), W.cols());
        N.setFromTriplets(triplets);
        N.swap(W);

        return true;
    }

private:
    bool sameAs(const NonLinear& other) const override {
        auto o = dynamic_cast<const GridBoxMaximum*>(&other);
        return o != nullptr;
    }

    void print(std::ostream& out) const override { out << "GridBoxMaximum[]"; }

    bool canIntroduceMissingValues() const override { return true; }

    void hash(eckit::MD5& h) const override {
        std::ostringstream s;
        s << *this;
        h.add(s.str());
    }
};


GridBoxMaximum::GridBoxMaximum(const param::MIRParametrisation& param) : GridBoxMethod(param) {
    addNonLinearTreatment(new NonLinearGridBoxMaximum(param));
}


static MethodBuilder<GridBoxMaximum> __builder("grid-box-maximum");


}  // namespace gridbox
}  // namespace method
}  // namespace mir
