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


#include "mir/method/Tessellation.h"
#include "atlas/mesh/generators/Delaunay.h"

namespace mir {
namespace method {

using atlas::mesh::generators::MeshGenerator;
using atlas::mesh::generators::Delaunay;

//----------------------------------------------------------------------------------------------------------------------


Tessellation::Tessellation(const param::MIRParametrisation &param) :
    FiniteElement(param) {
}


Tessellation::~Tessellation() {
}


const char *Tessellation::name() const {
    return  "tesselation";
}


void Tessellation::hash( eckit::MD5 &md5) const {
    FiniteElement::hash(md5);
}


void Tessellation::generateMesh(const atlas::grid::Grid &grid, atlas::mesh::Mesh &mesh) const {
    eckit::ScopedPtr<MeshGenerator> gen (new Delaunay());
    gen->generate(grid, mesh);
}

void Tessellation::print(std::ostream &out) const {
    out << "Tessellation[]";
}


namespace {
static MethodBuilder< Tessellation > __name("tessellation");
}

//----------------------------------------------------------------------------------------------------------------------


}  // namespace method
}  // namespace mir

