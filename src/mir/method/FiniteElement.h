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


#ifndef mir_method_FiniteElement_h
#define mir_method_FiniteElement_h

#include "eckit/geometry/Point3.h"
#include "eckit/memory/ScopedPtr.h"
#include "atlas/meshgenerator.h"
#include "mir/method/MethodWeighted.h"


namespace atlas {
class Grid;
class Mesh;
}


namespace mir {
namespace method {


class FiniteElement: public MethodWeighted {

public:

    typedef eckit::geometry::Point3 Point;

public:

    FiniteElement(const param::MIRParametrisation&);

    virtual ~FiniteElement();

protected:

    virtual void hash(eckit::MD5&) const;
    virtual void generateMesh(const atlas::Grid&, atlas::Mesh&) const;

protected: // methods

    // Deriving from any eckit::Parametrisation should work
    class MeshGenParams: public atlas::MeshGenerator::Parameters {
    public:
        MeshGenParams();
    };

    virtual void assemble(context::Context& ctx, WeightMatrix &W, const GridSpace& in, const GridSpace& out) const;

protected: // members

    MeshGenParams meshgenparams_;
};


}  // namespace method
}  // namespace mir


#endif

