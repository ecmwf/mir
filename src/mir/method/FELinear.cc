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


#include "mir/method/FELinear.h"

#include "mir/param/MIRParametrisation.h"
#include "atlas/mesh/generators/ReducedGridMeshGenerator.h"
#include "atlas/mesh/generators/Delaunay.h"
#include "atlas/grid/ReducedGrid.h"

namespace mir {
namespace method {


FELinear::FELinear(const param::MIRParametrisation &param) :
    FiniteElement(param) {
  meshgenparams_.set("triangulate", true); // No quads allowed
}


FELinear::~FELinear() {
}


const char *FELinear::name() const {
    return  "linear";
}


void FELinear::hash( eckit::MD5& md5) const {
    FiniteElement::hash(md5);
}

void FELinear::print(std::ostream &out) const {
    out << "FELinear[]";
}


namespace {
static MethodBuilder< FELinear > __linear("linear");
}


}  // namespace method
}  // namespace mir

