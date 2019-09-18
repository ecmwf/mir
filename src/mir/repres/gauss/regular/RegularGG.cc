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

#include "eckit/exception/Exceptions.h"

#include "mir/util/Domain.h"


namespace mir {
namespace repres {
namespace gauss {
namespace regular {


RegularGG::RegularGG(const param::MIRParametrisation& parametrisation) :
    Regular(parametrisation) {
}


RegularGG::RegularGG(size_t N, const util::BoundingBox& bbox, double angularPrecision):
    Regular(N, bbox, angularPrecision) {
}


RegularGG::~RegularGG() = default;


void RegularGG::print(std::ostream& out) const {
    out << "RegularGG["
        "N=" << N_
        << ",Ni=" << Ni_
        << ",Nj=" << Nj_
        << ",bbox=" << bbox_
        << "]";
}


bool RegularGG::sameAs(const Representation& other) const {
    auto o = dynamic_cast<const RegularGG*>(&other);
    return o && Regular::sameAs(other);
}


Iterator* RegularGG::iterator() const {
    auto Ni = [ = ](size_t) { return long(4 * N_); };
    return Gaussian::unrotatedIterator(Ni);
}


const Gridded* RegularGG::croppedRepresentation(const util::BoundingBox& bbox) const {
    return new RegularGG(N_, bbox, angularPrecision_);
}


std::string RegularGG::factory() const {
    return "regular_gg";
}


std::vector<double> RegularGG::calculateGridBoxLatitudeEdges() const {
    return calculateUnrotatedGridBoxLatitudeEdges();
}


std::vector<double> RegularGG::calculateGridBoxLongitudeEdges(size_t j) const {
    ASSERT(j < Nj_);
    ASSERT(1 < Ni_);

    eckit::Fraction half(1, 2);
    auto lon0 = bbox_.west();
    auto inc  = (bbox_.east() - bbox_.west()) / (Ni_ - 1);

    // grid-box longitude edges
    std::vector<double> edges(Ni_ + 1, 0.);
    edges[0] = (lon0 - inc / 2).value();
    for (size_t i = 0; i < Ni_; ++i) {
        edges[i + 1] = (lon0 + (i + half) * inc.fraction()).value();
    }

    return edges;
}


namespace {
static RepresentationBuilder<RegularGG> reducedGG("regular_gg"); // Name is what is returned by grib_api
}


}  // namespace regular
}  // namespace gauss
}  // namespace repres
}  // namespace mir

