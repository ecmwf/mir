// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/method/nonlinear/NoNonLinear.h"

#include <ostream>
#include <sstream>

#include "eckit/log/JSON.h"
#include "eckit/utils/MD5.h"


namespace mir::method::nonlinear {


NoNonLinear::NoNonLinear(const param::MIRParametrisation& param) : NonLinear(param) {}


bool NoNonLinear::treatment(DenseMatrix& /*A*/, WeightMatrix& /*W*/, DenseMatrix& /*B*/,
                            const MIRValuesVector& /*unused*/, const double& /*missingValue*/) const {
    // no non-linear treatment
    return false;
}


bool NoNonLinear::sameAs(const NonLinear& other) const {
    const auto* o = dynamic_cast<const NoNonLinear*>(&other);
    return (o != nullptr);
}


void NoNonLinear::print(std::ostream& out) const {
    out << "NoNonLinear[]";
}


void NoNonLinear::hash(eckit::MD5& h) const {
    std::ostringstream s;
    s << *this;
    h.add(s.str());
}


const std::string& NoNonLinear::name() const {
    static const std::string NAME{"no"};
    return NAME;
}


static const NonLinearBuilder<NoNonLinear> __nonlinear("no");


}  // namespace mir::method::nonlinear
