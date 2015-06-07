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
/// @author Pedro Maciel
/// @date May 2015


#ifndef mir_method_FiniteElement_H
#define mir_method_FiniteElement_H

#include "eckit/memory/ScopedPtr.h"
#include "eckit/geometry/Point3.h"

#include "atlas/Mesh.h"
#include "atlas/Field.h"
#include "atlas/PointIndex3.h"

#include "mir/method/MethodWeighted.h"

namespace mir {
namespace method {


class FiniteElement: public MethodWeighted {

  public:

    typedef eckit::geometry::Point3 Point;

  public:

    FiniteElement(const param::MIRParametrisation&);

    virtual ~FiniteElement();

  protected:

    virtual void hash( eckit::MD5& ) const;
    virtual void generateMesh(const atlas::Grid& g, atlas::Mesh& mesh) const;

  private:


// -- Overridden methods

    virtual void assemble(WeightMatrix& W, const atlas::Grid& in, const atlas::Grid& out) const;

};


}  // namespace method
}  // namespace mir

#endif

