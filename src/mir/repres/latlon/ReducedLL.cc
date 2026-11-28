// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/repres/latlon/ReducedLL.h"

#include <algorithm>
#include <cmath>
#include <ostream>
#include <sstream>

#include "eckit/log/JSON.h"
#include "eckit/spec/Custom.h"
#include "eckit/types/FloatCompare.h"
#include "eckit/utils/MD5.h"

#include "mir/api/MIRJob.h"
#include "mir/api/mir_config.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/Domain.h"
#include "mir/util/Exceptions.h"
#include "mir/util/GridBox.h"
#include "mir/util/MeshGeneratorParameters.h"
#include "mir/util/Types.h"


namespace mir::repres::latlon {

static bool checkPl(const std::vector<long>& pl) {
    ASSERT(!pl.empty());
    return *std::min_element(pl.begin(), pl.end()) >= 2;
}

ReducedLL::ReducedLL(const param::MIRParametrisation& parametrisation) : Gridded(parametrisation) {
    ASSERT(parametrisation.get("pl", pl_));
    checkPl(pl_);

    size_t Nj = 0;
    ASSERT(parametrisation.get("Nj", Nj));
    ASSERT(Nj == pl_.size());
}

ReducedLL::~ReducedLL() = default;

void ReducedLL::print(std::ostream& out) const {
    out << "ReducedLL[bbox=" << bbox_ << "]";
}

std::string ReducedLL::factory() const {
    return "reduced_ll";
}

void ReducedLL::json(eckit::JSON& s) const {
    s.startObject();
    s << "type"
      << "reduced_ll";
    s << "pl" << pl_;
    s << "bbox" << bbox_;
    s.endObject();
}

void ReducedLL::makeName(std::ostream& out) const {
    out << "RLL" << pl_.size() << "-";

    eckit::MD5 md5;
    for (const auto& j : pl_) {
        md5 << j;
    }

    out << std::string(md5);
    bbox_.makeName(out);
}

size_t ReducedLL::numberOfPoints() const {
    size_t total = 0;
    for (const auto& j : pl_) {
        total += size_t(j);
    }
    return total;
}

bool ReducedLL::getLongestElementDiagonal(double& d) const {

    // Look for a majorant of all element diagonals, using the difference of
    // latitudes closest/furthest from equator and longitude furthest from
    // Greenwich

    const util::Domain dom = domain();
    const bool periodic    = dom.isPeriodicWestEast();

    ASSERT(pl_.size() >= 2);
    const size_t Dj(pl_.size() - 1);
    ASSERT(Dj > 0);
    const auto sn = (dom.north() - dom.south()) / static_cast<double>(Dj);

    d         = 0.;
    auto lat1 = dom.north();
    auto lat2 = dom.north() - sn;

    for (size_t j = 1; j < pl_.size(); ++j) {

        const long Di(std::min(pl_[j - 1], pl_[j]) - (periodic ? 0 : 1));
        ASSERT(Di > 0);
        const auto we = (dom.east() - dom.west()) / static_cast<double>(Di);

        const auto latAwayFromEquator = std::abs(lat1) > std::abs(lat2) ? lat1 : lat2;
        const auto latCloserToEquator = std::abs(lat1) > std::abs(lat2) ? lat2 : lat1;

        d = std::max(d,
                     util::Earth::distance(PointLonLat{0., latCloserToEquator}, PointLonLat{we, latAwayFromEquator}));

        lat1 = lat2;
        lat2 -= sn;
    }

    ASSERT(d > 0.);
    return true;
}

bool ReducedLL::sameAs(const Representation& other) const {
    const auto* o = dynamic_cast<const ReducedLL*>(&other);
    return (o != nullptr) && (bbox_ == o->bbox_) && (pl_ == o->pl_);
}

void ReducedLL::fillGrib(grib_info& /*unused*/) const {
    NOTIMP;
}

void ReducedLL::fillSpec(CustomSpec& spec) const {
    Gridded::fillSpec(spec);

    spec.set("type", "reduced_ll");
    spec.set("pl", pl_);
}

atlas::Grid ReducedLL::atlasGrid() const {
    const util::Domain dom = domain();
    auto N                 = long(pl_.size());

    atlas::StructuredGrid::XSpace xspace({{dom.west(), dom.east()}}, pl_, !dom.isPeriodicWestEast());
    atlas::StructuredGrid::YSpace yspace(atlas::grid::LinearSpacing({{dom.north(), dom.south()}}, N));

    return atlas::StructuredGrid(xspace, yspace);
}

void ReducedLL::fillMeshGen(util::MeshGeneratorParameters& params) const {
    if (params.meshGenerator_.empty()) {
        params.meshGenerator_ = "structured";
    }
    if (eckit::types::is_strictly_greater(boundingBox().south(), 0., PointLonLat::EPS)) {
        params.set("force_include_south_pole", true);
    }
    if (eckit::types::is_strictly_greater(0., boundingBox().north(), PointLonLat::EPS)) {
        params.set("force_include_north_pole", true);
    }
}

bool ReducedLL::isPeriodicWestEast() const {
    ASSERT(!pl_.empty());

    auto maxpl = *std::max_element(pl_.begin(), pl_.end());
    ASSERT(maxpl >= 2);

    // if range West-East is within one increment (or greater than) 360 degree
    const auto inc = LONGITUDE_GLOBE / static_cast<double>(maxpl);
    return eckit::types::is_approximately_greater_or_equal(bbox_.east() - bbox_.west() + inc, LONGITUDE_GLOBE,
                                                           PointLonLat::EPS);
}

bool ReducedLL::includesNorthPole() const {
    return eckit::types::is_approximately_equal(bbox_.north(), NORTH_POLE, PointLonLat::EPS);
}

bool ReducedLL::includesSouthPole() const {
    return eckit::types::is_approximately_equal(bbox_.south(), SOUTH_POLE, PointLonLat::EPS);
}

void ReducedLL::validate(const MIRValuesVector& values) const {
    ASSERT_VALUES_SIZE_EQ_ITERATOR_COUNT("ReducedLL", values.size(), numberOfPoints());
}

std::vector<util::GridBox> ReducedLL::gridBoxes() const {
    const auto dom   = domain();
    const auto north = dom.north();
    const auto west  = dom.west();
    const auto south = dom.south();
    const auto east  = dom.east();

    const auto periodic = isPeriodicWestEast();

    auto Nj       = pl_.size();
    const auto sn = (north - south) / static_cast<double>(Nj - 1);


    // grid boxes
    std::vector<util::GridBox> r;
    r.reserve(numberOfPoints());


    // latitude edges
    std::vector<double> latEdges(Nj + 1);

    const auto lat0 = bbox_.north();
    latEdges[0]     = lat0 + sn / 2.;
    for (size_t j = 0; j < Nj; ++j) {
        latEdges[j + 1] = lat0 - (static_cast<double>(j) + 0.5) * sn;
    }

    latEdges.front() = std::min(north, std::max(south, latEdges.front()));
    if (dom.includesPoleNorth()) {
        latEdges.front() = NORTH_POLE;
    }

    latEdges.back() = std::min(north, std::max(south, latEdges.back()));
    if (dom.includesPoleSouth()) {
        latEdges.back() = SOUTH_POLE;
    }


    for (size_t j = 0; j < Nj; ++j) {

        // longitude edges
        auto Ni = pl_[j];
        ASSERT(Ni > 1);
        const auto we   = (east - west) / static_cast<double>(periodic ? Ni : Ni - 1);
        const auto lon0 = west - we / 2.;

        for (long i = 0; i < Ni; ++i) {
            auto w = lon0 + static_cast<double>(i) * we;
            auto e = lon0 + static_cast<double>(i + 1) * we;
            if (!periodic) {
                w = std::max(west, w);
                e = std::min(east, e);
            }
            r.emplace_back(latEdges[j], w, latEdges[j + 1], e);
        }
    }

    ASSERT(r.size() == numberOfPoints());
    return r;
}


const Representation* mir::repres::latlon::ReducedLL::croppedRepresentation(const util::BoundingBox&) const {
    std::ostringstream os;
    os << "ReducedLL::croppedRepresentation() not supported for " << *this;
    throw exception::FunctionalityNotSupported(os.str());
}


static const RepresentationBuilder<ReducedLL> reducedLL("reduced_ll");


}  // namespace mir::repres::latlon
