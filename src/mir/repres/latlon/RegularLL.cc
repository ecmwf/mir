// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/repres/latlon/RegularLL.h"

#include <ostream>

#include "eckit/geo/range/Regular.h"
#include "eckit/log/JSON.h"
#include "eckit/spec/Custom.h"

#include "mir/api/mir_config.h"
#include "mir/util/Atlas.h"
#include "mir/util/Domain.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Grib.h"
#include "mir/util/GridBox.h"


namespace mir::repres::latlon {

RegularLL::RegularLL(const param::MIRParametrisation& parametrisation) : LatLon(parametrisation) {}

RegularLL::RegularLL(const util::Increments& increments, const util::BoundingBox& bbox, const PointLatLon& reference) :
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

    const auto sn = eckit::Fraction(increments_.south_north());
    const auto we = eckit::Fraction(increments_.west_east());
    ASSERT(sn > 0);
    ASSERT(we > 0);

    // adjust West/East to include bbox's West/East ('outwards')
    Longitude w = bbox.west();
    if (increments_.isPeriodic()) {
        w = Regular::adjust(eckit::Fraction(bbox.west()), we, reference_.lon().fraction(), false, PRECISION);
    }
    Longitude e = Regular::adjust(eckit::Fraction(bbox.east()), we, reference_.lon().fraction(), true, PRECISION);


    // adjust South/North to include bbox's South/North ('outwards'), to the latitudes by the reference (South/West)
    // point, not beyond the poles: without a latitude between bbox's South/North and the pole, keep bbox's
    auto s = Regular::adjust(eckit::Fraction(bbox.south()), sn, reference_.lat().fraction(), false, PRECISION);
    if (s < Latitude::SOUTH_POLE.fraction()) {
        s = eckit::Fraction(bbox.south());
    }
    auto n = Regular::adjust(eckit::Fraction(bbox.north()), sn, reference_.lat().fraction(), true, PRECISION);
    if (n > Latitude::NORTH_POLE.fraction()) {
        n = eckit::Fraction(bbox.north());
    }

    // set bounding box
    const util::BoundingBox extended(n, w, s, e);
    ASSERT(extended.contains(bbox));

    return extended;
}

std::vector<util::GridBox> RegularLL::gridBoxes() const {
    const auto dom       = domain();
    const Latitude north = dom.north();
    const Longitude west = dom.west();
    const Latitude south = dom.south();
    const Longitude east = dom.east();

    const auto periodic = isPeriodicWestEast();

    Latitude lat0  = bbox_.north();
    Longitude lon0 = bbox_.west();
    auto sn        = Latitude(increments_.south_north());
    auto we        = eckit::Fraction(increments_.west_east());

    eckit::Fraction half(1, 2);


    // latitude edges
    std::vector<double> latEdges(nj_ + 1);

    latEdges[0] = (lat0 + sn / 2).value();
    for (size_t j = 0; j < nj_; ++j) {
        latEdges[j + 1] = (lat0 - (j + half) * sn.fraction()).value();
    }

    latEdges.front() = std::min(north.value(), std::max(south.value(), latEdges.front()));
    if (dom.includesPoleNorth()) {
        latEdges.front() = Latitude::NORTH_POLE.value();
    }

    latEdges.back() = std::min(north.value(), std::max(south.value(), latEdges.back()));
    latEdges.back() = std::min(north.value(), std::max(south.value(), latEdges.back()));
    if (dom.includesPoleSouth()) {
        latEdges.back() = Latitude::SOUTH_POLE.value();
    }


    // longitude edges
    std::vector<double> lonEdges(ni_ + 1);
    lonEdges[0] = (lon0 - we / 2).value();
    for (size_t i = 0; i < ni_; ++i) {
        lonEdges[i + 1] = (lon0 + (i + half) * we).value();
    }

    if (!periodic) {
        lonEdges.front() = std::max(west.value(), lonEdges.front());
        lonEdges.back()  = std::min(east.value(), lonEdges.back());
    }


    // grid boxes
    std::vector<util::GridBox> r;
    r.reserve(ni_ * nj_);

    for (size_t j = 0; j < nj_; ++j) {
        Longitude lon1 = lon0;

        for (size_t i = 0; i < ni_; ++i) {
            auto l = lon1;
            lon1   = l + we * (i + half);
            r.emplace_back(latEdges[j], lonEdges[i], latEdges[j + 1], lonEdges[i + 1]);
        }

        ASSERT(periodic ? lon0 == lon1.normalise(lon0) : lon0 <= lon1.normalise(lon0));
    }

    ASSERT(r.size() == numberOfPoints());
    return r;
}

std::string RegularLL::factory() const {
    return "regular_ll";
}


static const RepresentationBuilder<RegularLL> regularLL("regular_ll");


}  // namespace mir::repres::latlon
