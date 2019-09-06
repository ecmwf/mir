/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/method/gridbox/ConservativeGridBoxAverage.h"

#include <ostream>
#include <sstream>

#include "eckit/exception/Exceptions.h"
#include "eckit/utils/MD5.h"

#include "mir/method/WeightMatrix.h"
#include "mir/method/nonlinear/NonLinear.h"


namespace mir {
namespace method {
namespace gridbox {


struct GridBoxAverage : nonlinear::NonLinear {
    using NonLinear::NonLinear;
    bool treatment(Matrix& /*A*/, WeightMatrix& /*W*/, Matrix& /*B*/, const data::MIRValuesVector& /*values*/,
                   const double& /*missingValue*/) const {
        // TODO
        NOTIMP;
    }

private:
    bool sameAs(const NonLinear& other) const { return dynamic_cast<const GridBoxAverage*>(&other); }
    void print(std::ostream& out) const { out << "GridBoxAverage[]"; }
    void hash(eckit::MD5& h) const {
        std::ostringstream s;
        s << *this;
        h.add(s.str());
    }
};


ConservativeGridBoxAverage::ConservativeGridBoxAverage(const param::MIRParametrisation& param) : GridBoxMethod(param) {
    // TODO: MethodWeighted::pushNonLinear(new GridBoxAverage(param));  (currently MethodWeighted::missing_)
}


const char* ConservativeGridBoxAverage::name() const {
    return "conservative-grid-box-average";
}


static MethodBuilder<ConservativeGridBoxAverage> __builder("conservative-grid-box-average");


}  // namespace gridbox
}  // namespace method
}  // namespace mir
