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
/// @author Willem Deconinck
/// @date May 2015


#ifndef mir_method_FiniteElement_H
#define mir_method_FiniteElement_H

#include "eckit/memory/ScopedPtr.h"
#include "eckit/geometry/Point3.h"

#include "atlas/mesh/Mesh.h"
#include "atlas/field/Field.h"
#include "atlas/mesh/generators/MeshGenerator.h"

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
    virtual void generateMesh(const atlas::grid::Grid& g, atlas::mesh::Mesh& mesh) const;

  protected: // methods

    // Deriving from any eckit::Parametrisation should work
    class MeshGenParams: public atlas::mesh::generators::MeshGenerator::Parameters {
      public:
        MeshGenParams();
    };

    virtual void assemble(WeightMatrix& W, const atlas::grid::Grid& in, const atlas::grid::Grid& out) const;

  protected: // members

    MeshGenParams meshgenparams_;
};


}  // namespace method
}  // namespace mir

#endif

