/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @date Feb 2017


#include "mir/action/transform/ShVodTouvOctahedralGG.h"

#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/gauss/reduced/ReducedOctahedral.h"


namespace mir {
namespace action {
namespace transform {


ShVodTouvOctahedralGG::ShVodTouvOctahedralGG(const param::MIRParametrisation& parametrisation):
    ShVodTouvGridded(parametrisation) {

    ASSERT(parametrisation_.userParametrisation().get("octahedral", N_));

}


bool ShVodTouvOctahedralGG::sameAs(const Action& other) const {
    const ShVodTouvOctahedralGG* o = dynamic_cast<const ShVodTouvOctahedralGG*>(&other);
    return o && (N_ == o->N_);
}


ShVodTouvOctahedralGG::~ShVodTouvOctahedralGG() {
}


void ShVodTouvOctahedralGG::print(std::ostream& out) const {
    out << "ShVodTouvOctahedralGG[";
    ShToGridded::print(out);
    out << ",N=" << N_
        << "]";
}


const char* ShVodTouvOctahedralGG::name() const {
    return "ShVodTouvOctahedralGG";
}


const repres::Representation* ShVodTouvOctahedralGG::outputRepresentation() const {
    return new repres::gauss::reduced::ReducedOctahedral(N_);
}


namespace {
static ActionBuilder< ShVodTouvOctahedralGG > __action("transform.sh-vod-to-uv-octahedral-gg");
}


}  // namespace transform
}  // namespace action
}  // namespace mir

