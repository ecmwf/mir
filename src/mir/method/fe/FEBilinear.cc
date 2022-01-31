/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/method/fe/FEBilinear.h"


namespace mir {
namespace method {
namespace fe {


FEBilinear::FEBilinear(const param::MIRParametrisation& param, const std::string& label) : FiniteElement(param, label) {

    // generate meshes with triangles and perfect quadrilaterals
    // (adequate for the octahedral reduced Gaussian grid)
    meshGeneratorParams().set("triangulate", false).set("angle", 0.);
}


const char* FEBilinear::name() const {
    return "bilinear";
}


static const FiniteElementBuilder<FEBilinear> __builder("bilinear");


}  // namespace fe
}  // namespace method
}  // namespace mir
