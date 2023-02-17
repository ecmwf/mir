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


#include "mir/method/fe/FELinear.h"


namespace mir::method::fe {


FELinear::FELinear(const param::MIRParametrisation& param, const std::string& label) : FiniteElement(param, label) {

    // generate meshes only with triangles
    meshGeneratorParams().set("triangulate", true);
}


const char* FELinear::name() const {
    return "linear";
}


static const FiniteElementBuilder<FELinear> __builder("linear");


}  // namespace mir::method::fe
