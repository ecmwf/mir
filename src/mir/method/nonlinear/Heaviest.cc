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


#include "mir/method/nonlinear/Heaviest.h"

#include <ostream>
#include <sstream>

#include "eckit/log/JSON.h"
#include "eckit/utils/MD5.h"


namespace mir::method::nonlinear {


Heaviest::Heaviest(const param::MIRParametrisation& param) : NonLinear(param) {}


bool Heaviest::treatment(DenseMatrix& /*A*/, WeightMatrix& W, DenseMatrix& /*B*/, const MIRValuesVector& /*unused*/,
                         const double& /*missingValue*/) const {

    auto* data = const_cast<WeightMatrix::Scalar*>(W.data());

    WeightMatrix::iterator it(W);
    for (WeightMatrix::Size r = 0, i = 0; r < W.rows(); ++r) {

        // find heaviest-weighted column in row
        auto k                 = i;
        auto heaviest_index    = i;
        double heaviest_weight = -1.;
        size_t N_entries       = 0;

        const WeightMatrix::iterator end = W.end(r);
        for (; it != end; ++it, ++i, ++N_entries) {
            if (heaviest_weight < data[i]) {
                heaviest_weight = data[i];
                heaviest_index  = i;
            }
        }

        // set the heaviest-weighted column in row to 1, other entries to 0
        for (auto j = k; j < k + N_entries; ++j) {
            data[j] = j == heaviest_index ? 1. : 0.;
        }
    }

    return true;
}


bool Heaviest::sameAs(const NonLinear& other) const {
    const auto* o = dynamic_cast<const Heaviest*>(&other);
    return (o != nullptr);
}


void Heaviest::print(std::ostream& out) const {
    out << "Heaviest[]";
}


void Heaviest::hash(eckit::MD5& h) const {
    std::ostringstream s;
    s << *this;
    h.add(s.str());
}


void Heaviest::json(eckit::JSON& j) const {
    j.startObject();
    j << "type"
      << "heaviest";
    j.endObject();
}


static const NonLinearBuilder<Heaviest> __nonlinear("heaviest");


}  // namespace mir::method::nonlinear
