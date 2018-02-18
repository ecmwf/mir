/*
 * (C) Copyright 1996- ECMWF.
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


#include "mir/method/fe/FEBilinear.h"

#include "atlas/meshgenerator.h"


namespace mir {
namespace method {
namespace fe {


FEBilinear::FEBilinear(const param::MIRParametrisation &param) :
    FiniteElement(param) {

  inputMeshGenerationParams_.set("triangulate", false); // quads allowed

  // Only perfect quads are allowed, otherwise triangles (good for octahedral)
  // TODO: Find good default here, and allow override from MIRParametrisation
  inputMeshGenerationParams_.set("angle", 0.);
}


FEBilinear::~FEBilinear() {
}


bool FEBilinear::sameAs(const Method& other) const {
    const FEBilinear* o = dynamic_cast<const FEBilinear*>(&other);
    return o && FiniteElement::sameAs(other);
}


const char *FEBilinear::name() const {
    return  "bilinear";
}


void FEBilinear::hash( eckit::MD5 &md5) const {
    FiniteElement::hash(md5);
}


void FEBilinear::print(std::ostream &out) const {
    out << "FEBilinear[";
    FiniteElement::print(out);
    out << "]";
}


namespace {
static MethodBuilder< FEBilinear > __bilinear("bilinear");
}


}  // namespace fe
}  // namespace method
}  // namespace mir

