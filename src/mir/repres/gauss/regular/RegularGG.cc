// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/repres/gauss/regular/RegularGG.h"

#include <algorithm>
#include <ostream>
#include <utility>

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
    const auto north = dom.north();
    const auto west  = dom.west();
    const auto south = dom.south();
    const auto east  = dom.east();

    const auto periodic = isPeriodicWestEast();


    // longitude edges
    const auto lon0 = west;
    const auto we   = (east - west) / static_cast<double>(periodic ? Ni_ : Ni_ - 1);

    std::vector<double> lonEdges(Ni_ + 1);
    lonEdges[0] = lon0 - we / 2.;
    for (size_t i = 0; i < Ni_; ++i) {
        lonEdges[i + 1] = lon0 + (static_cast<double>(i) + 0.5) * we;
    }

    if (!periodic) {
        lonEdges.front() = std::max(west, lonEdges.front());
        lonEdges.back()  = std::min(east, lonEdges.back());
    }


    // grid boxes
    std::vector<util::GridBox> r;
    r.reserve(Ni_ * Nj_);

    for (size_t j = k_; j < k_ + Nj_; ++j) {
        for (size_t i = 0; i < Ni_; ++i) {
            r.emplace_back(std::min(north, latEdges[j]), lonEdges[i], std::max(south, latEdges[j + 1]),
                           lonEdges[i + 1]);
        }
    }

    ASSERT(r.size() == Ni_ * Nj_);
    return r;
}


static const RepresentationBuilder<RegularGG> reducedGG("regular_gg");


}  // namespace mir::repres::gauss::regular
