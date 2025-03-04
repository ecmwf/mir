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


#include "mir/repres/gauss/regular/RegularGG.h"

#include <algorithm>
#include <ostream>
#include <utility>

#include "mir/repres/gauss/GaussianIterator.h"
#include "mir/util/Domain.h"
#include "mir/util/Exceptions.h"
#include "mir/util/GridBox.h"


namespace mir::repres::gauss::regular {


RegularGG::RegularGG(const param::MIRParametrisation& parametrisation) : Regular(parametrisation) {}


RegularGG::RegularGG(size_t N, const util::BoundingBox& bbox, double angularPrecision) :
    Regular(N, bbox, angularPrecision) {}


void RegularGG::print(std::ostream& out) const {
    out << "RegularGG["
           "N="
        << N_ << ",Ni=" << Ni_ << ",Nj=" << Nj_ << ",bbox=" << bbox_ << "]";
}


bool RegularGG::sameAs(const Representation& other) const {
    const auto* o = dynamic_cast<const RegularGG*>(&other);
    return (o != nullptr) && Regular::sameAs(other);
}


Iterator* RegularGG::iterator() const {
    std::vector<long> pl(N_ * 2, long(4 * N_));
    return new gauss::GaussianIterator(latitudes(), std::move(pl), bbox_, N_, Nj_, k_);
}


const Gridded* RegularGG::croppedRepresentation(const util::BoundingBox& bbox) const {
    return new RegularGG(N_, bbox, angularPrecision_);
}


std::string RegularGG::factory() const {
    return "regular_gg";
}


std::vector<util::GridBox> RegularGG::gridBoxes() const {
    ASSERT(1 <= Ni_);
    ASSERT(1 <= Nj_);


    // latitude edges
    std::vector<double> latEdges = calculateUnrotatedGridBoxLatitudeEdges();

    const auto dom   = domain();
    const auto north = dom.north().value();
    const auto west  = dom.west();
    const auto south = dom.south().value();
    const auto east  = dom.east();

    const auto periodic = isPeriodicWestEast();


    // longitude edges
    auto lon0 = west;
    auto we   = (east - west).fraction() / (periodic ? Ni_ : Ni_ - 1);
    eckit::Fraction half(1, 2);

    std::vector<double> lonEdges(Ni_ + 1);
    lonEdges[0] = (lon0 - we / 2).value();
    for (size_t i = 0; i < Ni_; ++i) {
        lonEdges[i + 1] = (lon0 + (i + half) * we).value();
    }

    if (!periodic) {
        lonEdges.front() = std::max(west.value(), lonEdges.front());
        lonEdges.back()  = std::min(east.value(), lonEdges.back());
    }


    // grid boxes
    std::vector<util::GridBox> r;
    r.reserve(Ni_ * Nj_);

    for (size_t j = k_; j < k_ + Nj_; ++j) {
        Longitude lon1 = lon0;

        for (size_t i = 0; i < Ni_; ++i) {
            auto l = lon1;
            lon1   = l + Longitude(we * (i + half));
            r.emplace_back(std::min(north, latEdges[j]), lonEdges[i], std::max(south, latEdges[j + 1]),
                           lonEdges[i + 1]);
        }

        ASSERT(periodic ? lon0 == lon1.normalise(lon0) : lon0 <= lon1.normalise(lon0));
    }

    ASSERT(r.size() == Ni_ * Nj_);
    return r;
}


static const RepresentationBuilder<RegularGG> reducedGG("regular_gg");


}  // namespace mir::repres::gauss::regular
