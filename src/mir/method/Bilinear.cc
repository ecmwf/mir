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


#include "mir/method/Bilinear.h"

#include "atlas/meshgen/ReducedGridMeshGenerator.h"
#include "atlas/grids/ReducedGrid.h"

namespace mir {
namespace method {


Bilinear::Bilinear(const param::MIRParametrisation &param) :
    FiniteElement(param) {
}


Bilinear::~Bilinear() {
}


const char *Bilinear::name() const {
    return  "bilinear";
}


void Bilinear::hash( eckit::MD5 &md5) const {
    FiniteElement::hash(md5);
}


void Bilinear::generateMesh(const atlas::Grid &grid, atlas::Mesh &mesh) const {

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


    const atlas::grids::ReducedGrid *reduced = dynamic_cast<const atlas::grids::ReducedGrid *>(&grid);
    if (reduced == 0) {
        // Falling back to default behaviour
        FiniteElement::generateMesh(grid, mesh);
        return;
    }

    // fast tesselation method, specific for ReducedGrid's

    std::cout << "Mesh is ReducedGrid " << grid.shortName() << std::endl;

    atlas::meshgen::ReducedGridMeshGenerator generator;

    // force these flags
    generator.options.set("three_dimensional", true);
    generator.options.set("patch_pole", true);
    generator.options.set("include_pole", false);
    generator.options.set("triangulate", false);

    generator.generate(*reduced, mesh);

}


void Bilinear::print(std::ostream &out) const {
    out << "Bilinear[]";
}


namespace {
static MethodBuilder< Bilinear > __bilinear("bilinear");
}


}  // namespace method
}  // namespace mir

