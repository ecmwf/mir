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


#include "mir/method/nonlinear/NoNonLinear.h"

#include <ostream>
#include <sstream>

#include "eckit/log/JSON.h"
#include "eckit/utils/MD5.h"


namespace mir::method::nonlinear {


NoNonLinear::NoNonLinear(const param::MIRParametrisation& param) : NonLinear(param) {}


bool NoNonLinear::treatment(MethodWeighted::Matrix& /*A*/, MethodWeighted::WeightMatrix& /*W*/,
                            MethodWeighted::Matrix& /*B*/, const MIRValuesVector& /*unused*/,
                            const double& /*missingValue*/) const {
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


void NoNonLinear::json(eckit::JSON& j) const {
    j.startObject();
    j << "type" << "no";
    j.endObject();
}


static const NonLinearBuilder<NoNonLinear> __nonlinear("no");


}  // namespace mir::method::nonlinear
