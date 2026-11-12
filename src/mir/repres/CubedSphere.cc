// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/repres/CubedSphere.h"

#include <ostream>
#include <string>

#include "eckit/geo/grid/CubedSphere.h"
#include "eckit/log/JSON.h"
#include "eckit/spec/Custom.h"

#include "mir/iterator/UnstructuredIterator.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/Atlas.h"
#include "mir/util/BoundingBox.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Grib.h"
#include "mir/util/MeshGeneratorParameters.h"


namespace mir::repres {


static const RepresentationBuilder<CubedSphere> CUBED_SPHERE("cubed_sphere");


CubedSphere::CubedSphere(const param::MIRParametrisation& param) :
    CubedSphere([&param]() {
        std::string gridspec;
        ASSERT(param.get("gridspec", gridspec));

        std::unique_ptr<const eckit::geo::Grid> grid(eckit::geo::GridFactory::make_from_string(gridspec));
        if (const auto* cs = dynamic_cast<const eckit::geo::grid::CubedSphere*>(grid.get()); cs != nullptr) {
            grid.release();
            return cs;
        }

        throw exception::UserError("CubedSphere: gridspec is not a cubed sphere: '" + gridspec + "'");
    }()) {}


CubedSphere::CubedSphere(const eckit::geo::grid::CubedSphere* grid) : Gridded(util::BoundingBox()), grid_(grid) {
    ASSERT(grid_);
}


CubedSphere::~CubedSphere() = default;


const CubedSphere::points_type& CubedSphere::to_latlons() const {
    if (points_.first.empty() || points_.second.empty()) {
        ASSERT(points_.first.empty() && points_.second.empty());

        points_ = grid_->to_latlons();
        ASSERT(points_.first.size() == points_.second.size());
        ASSERT(points_.first.size() == numberOfPoints());
    }

    return points_;
}


bool CubedSphere::sameAs(const Representation& other) const {
    const auto* o = dynamic_cast<const CubedSphere*>(&other);
    return (o != nullptr) && *grid_ == *(o->grid_);
}


void CubedSphere::makeName(std::ostream& out) const {
    out << grid_->type() << '-' << numberOfPoints() << '-' << grid_->uid();
}


void CubedSphere::fillGrib(grib_info& info) const {
    // no GRIB cubed sphere template, using unstructured
    info.grid.grid_type        = GRIB_UTIL_GRID_SPEC_UNSTRUCTURED;
    info.packing.editionNumber = 2;

    info.extra_set("uuidOfHGrid", grid_->uid().c_str());
}


void CubedSphere::fillMeshGen(util::MeshGeneratorParameters& params) const {
    if (params.meshGenerator_.empty()) {
        params.meshGenerator_ = "delaunay";
    }
}


void CubedSphere::fillSpec(CustomSpec& spec) const {
    spec.set(dynamic_cast<const eckit::spec::Custom&>(grid_->spec()));
}


void CubedSphere::json(eckit::JSON& j) const {
    grid_->spec().json(j);
}


void CubedSphere::print(std::ostream& out) const {
    out << "CubedSphere[" << grid_->spec_str() << "]";
}


void CubedSphere::validate(const MIRValuesVector& values) const {
    ASSERT_VALUES_SIZE_EQ_ITERATOR_COUNT("CubedSphere", values.size(), numberOfPoints());
}


size_t CubedSphere::numberOfPoints() const {
    return grid_->size();
}


Iterator* CubedSphere::iterator() const {
    const auto& [lats, lons] = to_latlons();
    return new iterator::UnstructuredIterator(lats, lons);
}


::atlas::Grid CubedSphere::atlasGrid() const {
    const auto& [lats, lons] = to_latlons();

    std::vector<atlas::PointXY> points(lats.size());
    for (size_t i = 0; i < points.size(); ++i) {
        points[i].assign(lons[i], lats[i]);
    }

    return atlas::UnstructuredGrid(std::move(points));
}


}  // namespace mir::repres
