// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/repres/latlon/RotatedLL.h"

#include <ostream>

#include "eckit/spec/Custom.h"

#include "mir/api/mir_config.h"
#include "mir/iterator/GridIterator.h"
#include "mir/util/Atlas.h"
#include "mir/util/Domain.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Grib.h"


namespace mir::repres::latlon {

RotatedLL::RotatedLL(const param::MIRParametrisation& parametrisation) :
    LatLon(parametrisation), rotation_(parametrisation) {}

RotatedLL::RotatedLL(const util::Increments& increments, const util::Rotation& rotation, const util::BoundingBox& bbox,
                     const PointLatLon& reference) :
    LatLon(increments, bbox, reference), rotation_(rotation) {}

RotatedLL::~RotatedLL() = default;

Iterator* RotatedLL::iterator() const {
    return new iterator::GridIterator(grid(), true);
}

void RotatedLL::print(std::ostream& out) const {
    out << "RotatedLL[";
    LatLon::print(out);
    out << ",rotation=" << rotation_ << "]";
}

atlas::Grid RotatedLL::atlasGrid() const {
    // NOTE: yspace uses bounding box and not the domain
    // (this works together with the Atlas RectangularDomain cropping)
    const util::Domain dom = domain();

    atlas::StructuredGrid::XSpace xspace(
        atlas::grid::LinearSpacing(dom.west().value(), dom.east().value(), long(ni_), !dom.isPeriodicWestEast()));

    atlas::StructuredGrid::YSpace yspace(
        atlas::grid::LinearSpacing(bbox_.north().value(), bbox_.south().value(), long(nj_)));

    atlas::StructuredGrid grid(xspace, yspace, {}, dom);
    return rotate_atlas_grid(rotation_.rotation(), grid);
}

void RotatedLL::fillGrib(grib_info& info) const {
    LatLon::fillGrib(info);

    info.grid.grid_type = CODES_UTIL_GRID_SPEC_ROTATED_LL;
    rotation_.fillGrib(info);
}

void RotatedLL::fillSpec(CustomSpec& spec) const {
    LatLon::fillSpec(spec);
    rotation_.fillSpec(spec);
}

void RotatedLL::makeName(std::ostream& out) const {
    LatLon::makeName(out);
    rotation_.makeName(out);
}

bool RotatedLL::sameAs(const Representation& other) const {
    const auto* o = dynamic_cast<const RotatedLL*>(&other);
    return (o != nullptr) && (rotation_ == o->rotation_) && LatLon::sameAs(other);
}

const RotatedLL* RotatedLL::croppedRepresentation(const util::BoundingBox& bbox) const {
    return new RotatedLL(increments_, rotation_, bbox, reference_);
}


util::Domain RotatedLL::geographicDomain() const {
    return rotation_.domain(domain());
}


std::string RotatedLL::factory() const {
    return "rotated_ll";
}


static const RepresentationBuilder<RotatedLL> rotatedLL("rotated_ll");


}  // namespace mir::repres::latlon
