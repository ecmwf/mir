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
/// @date   July 2015

#ifndef mir_method_Conservative_h
#define mir_method_Conservative_h

#include "mir/method/FELinear.h"

namespace eckit { namespace linalg { class Vector; } }
namespace atlas { class Mesh; }

namespace mir {
namespace method {


class Conservative: public FELinear {

public:

    Conservative(const param::MIRParametrisation&);

    virtual ~Conservative();

protected:

    virtual void hash(eckit::MD5&) const;

    virtual void assemble(context::Context& ctx, WeightMatrix &W, const GridSpace& in, const GridSpace& out) const;

    void computeLumpedMassMatrix(eckit::linalg::Vector&, const atlas::Grid&, atlas::Mesh&) const;

private:

  virtual void print(std::ostream&) const;
  virtual const char* name() const;

};


}  // namespace method
}  // namespace mir


#endif

