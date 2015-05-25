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


#include "mir/method/Linear.h"

#include "atlas/meshgen/ReducedGridMeshGenerator.h"
#include "atlas/meshgen/Delaunay.h"
#include "atlas/grids/ReducedGrid.h"

namespace mir {
namespace method {


Linear::Linear(const param::MIRParametrisation &param) :
    FiniteElement(param) {
}


Linear::~Linear() {
}


const char *Linear::name() const {
    return  "bilinear";
}


void Linear::hash( eckit::MD5& md5) const {
    FiniteElement::hash(md5);
}


void Linear::generateMesh(const atlas::Grid& g, atlas::Mesh& mesh) const
{


    std::cout << "Mesh not in cache -- tesselating grid " << g.unique_id() << std::endl;

    /// @TODO Ask Baudouin best way to build and parametrize the mesh generator
    ///       MeshGenerator is in Atlas -- should we bring to MIR ??
    ///       If stays in Atlas, we cannot pass MirParametrisation
    ///
    ///  This raises another issue: hoe to cache meshes generated with different parametrizations?
    ///  We must md5 the MeshGenerator itself.
    ///
    ///  We should be using something like:
    ///
    //    std::string meshGenerator;
    //    ASSERT(parametrisation_.get("meshGenerator", meshGenerator));
    //    eckit::ScopedPtr<MeshGenerator> meshGen( MeshGeneratorFactory::build(meshGenerator) );
    //    meshGen->tesselate(in, i_mesh);


    const atlas::grids::ReducedGrid* rg = dynamic_cast<const atlas::grids::ReducedGrid*>(&g);
    if (rg) {

      // fast tesselation method, specific for ReducedGrid's

      std::cout << "Mesh is ReducedGrid " << g.shortName() << std::endl;

      atlas::meshgen::ReducedGridMeshGenerator mg;

      // force these flags
      mg.options.set("three_dimensional", true);
      mg.options.set("patch_pole", true);
      mg.options.set("include_pole", false);
      mg.options.set("triangulate", true);

      mg.generate(*rg, mesh);

    } else {

      // slower, more robust tesselation method, using Delaunay triangulation

      std::cout << "Using Delaunay triangulation on grid: " << g.shortName() << std::endl;

      atlas::meshgen::Delaunay mg;
      mg.tesselate(g, mesh);
    }

}



void Linear::print(std::ostream &out) const {
    out << "Linear[]";
}


namespace {
static MethodBuilder< Linear > __linear("linear");
}


}  // namespace method
}  // namespace mir

