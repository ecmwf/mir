// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/repres/RegularXY.h"

#include <ostream>
#include <string>

#include "eckit/geo/Grid.h"
#include "eckit/geo/area/BoundingBox.h"
#include "eckit/log/JSON.h"
#include "eckit/spec/Custom.h"
#include "eckit/types/FloatCompare.h"

#include "mir/iterator/UnstructuredIterator.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/Atlas.h"
#include "mir/util/BoundingBox.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Grib.h"
#include "mir/util/MeshGeneratorParameters.h"


namespace mir::repres {


static const RepresentationBuilder<RegularXY> REGULAR_XY("regular_xy");


RegularXY::RegularXY(const param::MIRParametrisation& param) :
    RegularXY([&param]() {
        std::string gridspec;
        ASSERT(param.get("gridspec", gridspec));
        return eckit::geo::GridFactory::make_from_string(gridspec);
    }()) {}


RegularXY::RegularXY(const eckit::geo::Grid* grid) :
    Gridded([grid]() {
        ASSERT(grid != nullptr);
        auto [n, w, s, e] = grid->boundingBox().deconstruct();
        return util::BoundingBox{n, w, s, e};
    }()),
    grid_(grid) {
    ASSERT(grid_);
}


RegularXY::~RegularXY() = default;


const RegularXY::points_type& RegularXY::to_latlons() const {
    if (points_.first.empty() || points_.second.empty()) {
        ASSERT(points_.first.empty() && points_.second.empty());

        points_ = grid_->to_latlons();
        ASSERT(points_.first.size() == points_.second.size());
        ASSERT(points_.first.size() == numberOfPoints());
    }

    return points_;
}


bool RegularXY::sameAs(const Representation& other) const {
    const auto* o = dynamic_cast<const RegularXY*>(&other);
    return (o != nullptr) && *grid_ == *(o->grid_);
}


void RegularXY::makeName(std::ostream& out) const {
    out << grid_->type() << '-' << numberOfPoints() << '-' << grid_->uid();
}


void RegularXY::fillGrib(grib_info& info) const {
    info.grid.grid_type        = GRIB_UTIL_GRID_SPEC_UNSTRUCTURED;
    info.packing.editionNumber = 2;
}


void RegularXY::fillMeshGen(util::MeshGeneratorParameters& params) const {
    if (params.meshGenerator_.empty()) {
        params.meshGenerator_ = "delaunay";
    }
}


void RegularXY::fillSpec(CustomSpec& spec) const {
    spec.set(dynamic_cast<const eckit::spec::Custom&>(grid_->spec()));
}


void RegularXY::json(eckit::JSON& j) const {
    grid_->spec().json(j);
}


void RegularXY::print(std::ostream& out) const {
    out << "RegularXY[" << grid_->spec_str() << "]";
}


void RegularXY::validate(const MIRValuesVector& values) const {
    ASSERT_VALUES_SIZE_EQ_ITERATOR_COUNT("RegularXY", values.size(), numberOfPoints());
}


size_t RegularXY::numberOfPoints() const {
    return grid_->size();
}


Iterator* RegularXY::iterator() const {
    const auto& [lats, lons] = to_latlons();
    return new iterator::UnstructuredIterator(lats, lons);
}


bool RegularXY::includesNorthPole() const {
    return bbox_.north() == Latitude::NORTH_POLE;
}


bool RegularXY::includesSouthPole() const {
    return bbox_.south() == Latitude::SOUTH_POLE;
}


bool RegularXY::isPeriodicWestEast() const {
    return eckit::types::is_approximately_greater_or_equal(bbox_.east().value() - bbox_.west().value(),
                                                           Longitude::GLOBE.value());
}


::atlas::Grid RegularXY::atlasGrid() const {
    const auto& [lats, lons] = to_latlons();
    ASSERT(!lats.empty());
    ASSERT(lats.size() == lons.size());

    const auto N = lats.size();
    std::vector<atlas::PointXY> points(N);
    for (size_t i = 0; i < N; ++i) {
        points[i].assign(lons[i], lats[i]);
    }

    return atlas::UnstructuredGrid(std::move(points));
}


}  // namespace mir::repres
