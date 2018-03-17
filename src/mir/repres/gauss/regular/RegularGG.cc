/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date Apr 2015


#include "mir/repres/gauss/regular/RegularGG.h"

#include <iostream>
#include "eckit/memory/ScopedPtr.h"
#include "mir/action/misc/AreaCropper.h"
#include "mir/util/Domain.h"


namespace mir {
namespace repres {
namespace gauss {
namespace regular {


RegularGG::RegularGG(const param::MIRParametrisation& parametrisation):
    Regular(parametrisation) {
}


RegularGG::RegularGG(size_t N):
    Regular(N) {
}


RegularGG::RegularGG(size_t N, const util::BoundingBox& bbox, bool correctBoundingBox):
    Regular(N, bbox, correctBoundingBox) {
}


RegularGG::~RegularGG() {
}


void RegularGG::print(std::ostream& out) const {
    out << "RegularGG[N" << N_ << ",bbox=" << bbox_ << "]";
}


void RegularGG::makeName(std::ostream& out) const {
    Regular::makeName(out);
}


bool RegularGG::sameAs(const Representation& other) const {
    const RegularGG* o = dynamic_cast<const RegularGG*>(&other);
    return o && Regular::sameAs(other);
}


Iterator* RegularGG::iterator() const {
    auto Ni = [=](size_t){ return long(4 * N_); };
    return Gaussian::unrotatedIterator(Ni);
}


const Gridded* RegularGG::croppedRepresentation(const util::BoundingBox& bbox) const {
    return new RegularGG(N_, bbox, false);
}


namespace {
static RepresentationBuilder<RegularGG> reducedGG("regular_gg"); // Name is what is returned by grib_api
}


}  // namespace regular
}  // namespace gauss
}  // namespace repres
}  // namespace mir

