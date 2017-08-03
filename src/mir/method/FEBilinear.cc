/*
 * (C) Copyright 1996-2016 ECMWF.
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


#include "mir/method/FEBilinear.h"

#include "atlas/meshgenerator.h"


namespace mir {
namespace method {


FEBilinear::FEBilinear(const param::MIRParametrisation &param) :
    FiniteElement(param) {

  InputMeshGenerationParams_.set("triangulate", false); // quads allowed

  // Only perfect quads are allowed, otherwise triangles (good for octahedral)
  // TODO: Find good default here, and allow override from MIRParametrisation
  InputMeshGenerationParams_.set("angle", 0.);
}


FEBilinear::~FEBilinear() {
}


const char *FEBilinear::name() const {
    return  "fe-bilinear";
}


void FEBilinear::hash( eckit::MD5 &md5) const {
    FiniteElement::hash(md5);
}


void FEBilinear::print(std::ostream &out) const {
    out << "FEBilinear[]";
}


namespace {
static MethodBuilder< FEBilinear > __bilinear("fe-bilinear");
}


}  // namespace method
}  // namespace mir

