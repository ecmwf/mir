/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Tiago Quintino
/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date   July 2015


#ifndef mir_method_fe_Conservative_h
#define mir_method_fe_Conservative_h

#include "mir/method/fe/FELinear.h"


namespace eckit {
namespace linalg {
class Vector;
}
}


namespace mir {
namespace method {
namespace fe {


class Conservative: public FELinear {

public:

    Conservative(const param::MIRParametrisation&);

protected:

    void hash(eckit::MD5&) const;

    void assemble(util::MIRStatistics&,
                  WeightMatrix&,
                  const repres::Representation& in,
                  const repres::Representation& out) const;

    void computeLumpedMassMatrix(eckit::linalg::Vector&, const atlas::Mesh&) const;

private:

    void print(std::ostream&) const;
    const char* name() const;

};


}  // namespace fe
}  // namespace method
}  // namespace mir


#endif

