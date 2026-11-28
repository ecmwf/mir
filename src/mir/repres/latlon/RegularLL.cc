// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/repres/latlon/RegularLL.h"

#include <ostream>

#include "eckit/geo/range/Regular.h"
#include "eckit/log/JSON.h"
#include "eckit/spec/Custom.h"
#include "eckit/types/FloatCompare.h"

#include "mir/api/mir_config.h"
#include "mir/util/Atlas.h"
#include "mir/util/Domain.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Grib.h"
#include "mir/util/GridBox.h"


namespace mir::repres::latlon {

RegularLL::RegularLL(const param::MIRParametrisation& parametrisation) : LatLon(parametrisation) {}

RegularLL::RegularLL(const util::Increments& increments, const util::BoundingBox& bbox, const PointLonLat& reference) :
    LatLon(increments, bbox, reference) {}

RegularLL::~RegularLL() = default;

void RegularLL::print(std::ostream& out) const {
    out << "RegularLL[";
    LatLon::print(out);
    out << "]";
}

void RegularLL::json(eckit::JSON& json) const {
    json.startObject();
    json << "type"
         << "regular_ll";
    LatLon::json(json);
    json.endObject();
}

atlas::Grid RegularLL::atlasGrid() const {
    // NOTE: yspace uses bounding box and not the domain
    // (this works together with the Atlas RectangularDomain cropping)
    const auto dom = domain();

    atlas::StructuredGrid::XSpace xspace(
        atlas::grid::LinearSpacing(dom.west(), dom.east(), long(ni_), !dom.isPeriodicWestEast()));

    atlas::StructuredGrid::YSpace yspace(atlas::grid::LinearSpacing(bbox_.north(), bbox_.south(), long(nj_)));

    return atlas::StructuredGrid(xspace, yspace, {}, dom);
}

void RegularLL::fillGrib(grib_info& info) const {
    LatLon::fillGrib(info);

    // See copy_spec_from_ksec.c in libemos for info
    // Warning: scanning mode not considered
    info.grid.grid_type = CODES_UTIL_GRID_SPEC_REGULAR_LL;
}

void RegularLL::fillSpec(CustomSpec& spec) const {
    LatLon::fillSpec(spec);
}

void RegularLL::makeName(std::ostream& out) const {
    LatLon::makeName(out);
}

bool RegularLL::sameAs(const Representation& other) const {
    const auto* o = dynamic_cast<const RegularLL*>(&other);
    return (o != nullptr) && LatLon::sameAs(other);
}

const RegularLL* RegularLL::croppedRepresentation(const util::BoundingBox& bbox) const {
    return new RegularLL(increments_, bbox, reference_);
}

util::BoundingBox RegularLL::extendBoundingBox(const util::BoundingBox& bbox) const {
    using eckit::geo::range::Regular;

    const auto sn = increments_.south_north();
    const auto we = increments_.west_east();
    ASSERT(sn > 0);
    ASSERT(we > 0);

    // adjust West/East to include bbox's West/East ('outwards')
    auto w = bbox.west();
    if (increments_.isPeriodic()) {
        w = Regular::adjust(bbox.west(), we, reference_.lon(), false, PRECISION);
    }
    const auto e = Regular::adjust(bbox.east(), we, reference_.lon(), true, PRECISION);


    // adjust South/North to include bbox's South/North ('outwards'), to the latitudes by the reference (South/West)
    // point, not beyond the poles: without a latitude between bbox's South/North and the pole, keep bbox's
    auto s = Regular::adjust(bbox.south(), sn, reference_.lat(), false, PRECISION);
    if (eckit::types::is_strictly_greater(SOUTH_POLE, s, PointLonLat::EPS)) {
        s = bbox.south();
    }
    auto n = Regular::adjust(bbox.north(), sn, reference_.lat(), true, PRECISION);
    if (eckit::types::is_strictly_greater(n, NORTH_POLE, PointLonLat::EPS)) {
        n = bbox.north();
    }

    // set bounding box
    const util::BoundingBox extended(n, w, s, e);
    ASSERT(extended.contains(bbox));

    return extended;
}

std::vector<util::GridBox> RegularLL::gridBoxes() const {
    const auto dom   = domain();
    const auto north = dom.north();
    const auto west  = dom.west();
    const auto south = dom.south();
    const auto east  = dom.east();

    const auto periodic = isPeriodicWestEast();

    const auto lat0 = bbox_.north();
    const auto lon0 = bbox_.west();
    const auto sn   = increments_.south_north();
    const auto we   = increments_.west_east();


    // latitude edges
    std::vector<double> latEdges(nj_ + 1);

    latEdges[0] = lat0 + sn / 2.;
    for (size_t j = 0; j < nj_; ++j) {
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


    // longitude edges
    std::vector<double> lonEdges(ni_ + 1);
    lonEdges[0] = lon0 - we / 2.;
    for (size_t i = 0; i < ni_; ++i) {
        lonEdges[i + 1] = lon0 + (static_cast<double>(i) + 0.5) * we;
    }

    if (!periodic) {
        lonEdges.front() = std::max(west, lonEdges.front());
        lonEdges.back()  = std::min(east, lonEdges.back());
    }


    // grid boxes
    std::vector<util::GridBox> r;
    r.reserve(ni_ * nj_);

    for (size_t j = 0; j < nj_; ++j) {
        for (size_t i = 0; i < ni_; ++i) {
            r.emplace_back(latEdges[j], lonEdges[i], latEdges[j + 1], lonEdges[i + 1]);
        }
    }

    ASSERT(r.size() == numberOfPoints());
    return r;
}

std::string RegularLL::factory() const {
    return "regular_ll";
}


static const RepresentationBuilder<RegularLL> regularLL("regular_ll");


}  // namespace mir::repres::latlon
