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


#include "mir/method/fe/FELinear.h"

#include "atlas/meshgenerator.h"
#include "mir/param/MIRParametrisation.h"


namespace mir {
namespace method {
namespace fe {


FELinear::FELinear(const param::MIRParametrisation &param) :
    FiniteElement(param) {
    inputMeshGenerationParams_.set("triangulate", true); // No quads allowed
}


FELinear::~FELinear() {
}


bool FELinear::sameAs(const Method& other) const {
    const FELinear* o = dynamic_cast<const FELinear*>(&other);
    return o && FiniteElement::sameAs(other);
}

const char *FELinear::name() const {
    return  "linear";
}


void FELinear::hash( eckit::MD5& md5) const {
    FiniteElement::hash(md5);
}


void FELinear::print(std::ostream &out) const {
    out << "FELinear[";
    FiniteElement::print(out);
    out << "]";
}


namespace {
static MethodBuilder< FELinear > __linear("linear");
}


}  // namespace fe
}  // namespace method
}  // namespace mir

