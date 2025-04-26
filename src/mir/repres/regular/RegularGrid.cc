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


#include "mir/repres/regular/RegularGrid.h"

#include <algorithm>
#include <memory>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

#include "eckit/config/Resource.h"
#include "eckit/geo/area/BoundingBox.h"
#include "eckit/geo/grid/RegularXY.h"
#include "eckit/geo/range/RegularCartesian.h"
#include "eckit/geo/spec/Custom.h"
#include "eckit/utils/MD5.h"
#include "eckit/utils/StringTools.h"

#include "mir/api/MIRJob.h"
#include "mir/iterator/UnstructuredIterator.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Iterator.h"
#include "mir/util/Domain.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Grib.h"
#include "mir/util/Log.h"
#include "mir/util/MeshGeneratorParameters.h"


namespace mir::repres::regular {


RegularGrid::RegularGrid(const param::MIRParametrisation& param, Projection* projection) :
    firstPointBottomLeft_(false) {
    ASSERT(projection != nullptr);

    auto get_long_first_key = [&param](const std::vector<std::string>& keys) -> long {
        for (const auto& key : keys) {
            if (long value = 0; param.get(key, value)) {
                return value;
            }
        }
        throw exception::SeriousBug("RegularGrid: couldn't find any key: " + eckit::StringTools::join(", ", keys));
    };

    auto get_double = [&param](const std::string& key) -> double {
        if (double value = 0; param.get(key, value)) {
            return value;
        }
        throw exception::SeriousBug("RegularGrid: couldn't find key: " + key);
    };

    long nx = get_long_first_key({"numberOfPointsAlongXAxis", "Nx", "Ni"});
    long ny = get_long_first_key({"numberOfPointsAlongYAxis", "Ny", "Nj"});
    ASSERT(nx > 0);
    ASSERT(ny > 0);

    std::vector<double> grid;
    ASSERT(param.get("grid", grid));
    ASSERT_KEYWORD_GRID_SIZE(grid.size());

    PointLonLat firstLL{get_double("longitudeOfFirstGridPointInDegrees"),
                        get_double("latitudeOfFirstGridPointInDegrees")};
    auto first = std::get<PointXY>(projection->fwd(firstLL));

    bool xPlus_ = true;
    bool yPlus_ = false;
    param.get("iScansPositively", xPlus_);  // iScansPositively != 0
    param.get("jScansPositively", yPlus_);  // jScansPositively == 0
    param.get("first_point_bottom_left", firstPointBottomLeft_);


    auto linspace = [](double start, double step, long num, bool plus) -> ::eckit::geo::Range* {
        ASSERT(step >= 0.);
        ASSERT(num > 1);

        return new ::eckit::geo::range::RegularCartesian{static_cast<size_t>(num), start,
                                                         start + step * static_cast<double>(plus ? num - 1 : 1 - num)};
    };

    grid_ = std::make_unique<eckit::geo::grid::RegularXY>(
        linspace(first.X, grid[0], nx, firstPointBottomLeft_ || xPlus_),
        linspace(first.Y, grid[1], ny, firstPointBottomLeft_ || yPlus_), projection);

    PointXY min{grid_->x().min(), grid_->y().min()};
    PointXY max{grid_->x().max(), grid_->y().max()};

    std::unique_ptr<::eckit::geo::area::BoundingBox> bbox(
        ::eckit::geo::area::BoundingBox::make_from_projection(min, max, *projection));


    // MIR-661 Grid projection handling covering the poles: account for "excessive" bounds
    auto west(bbox->west);
    auto east = bbox->east - bbox->west >= PointLonLat::FULL_ANGLE ? west + PointLonLat::FULL_ANGLE : bbox->east;

    bbox_ = {bbox->north, west, bbox->south, east};
}


RegularGrid::RegularGrid(Grid* grid, const util::BoundingBox& bbox, const util::Shape& shape) :
    Gridded(bbox), grid_(grid), shape_(shape), firstPointBottomLeft_(false) {
    ASSERT(grid != nullptr);
    if (!shape_.provided) {
        shape_ = util::Shape{grid_->projection().spec()};
    }
}


RegularGrid::~RegularGrid() = default;


RegularGrid::Projection* RegularGrid::make_projection(const param::MIRParametrisation& param) {
    static bool useProjIfAvailable = eckit::Resource<bool>("$MIR_USE_PROJ_IF_AVAILABLE", true);

    std::string proj;
    param.get("proj", proj);

    if (proj.empty()) {
        return {};
    }

    ::eckit::geo::spec::Custom spec{{"type", "proj"}, {"proj", proj}};

    if (std::string proj; param.get("projSource", proj) && !proj.empty()) {
        spec.set("proj_source", proj);
    }

    if (std::string proj; param.get("projGeocentric", proj) && !proj.empty()) {
        spec.set("proj_geocentric", proj);
    }

    return eckit::geo::ProjectionFactory::build(spec);
}


void RegularGrid::print(std::ostream& out) const {
    out << "RegularGrid[grid=" << grid_->spec_str() << ",firstPointBottomLeft=" << firstPointBottomLeft_
        << ",bbox=" << bbox_ << "]";
}


bool RegularGrid::extendBoundingBoxOnIntersect() const {
    return true;
}


size_t RegularGrid::numberOfPoints() const {
    return x().size() * y().size();
}


::atlas::Grid RegularGrid::atlasGrid() const {
    NOTIMP;
}


void RegularGrid::fillGrib(grib_info& info) const {
    // shape of the reference system
    shape_.fillGrib(info, grid_->projection().spec());

    // scanningMode
    info.grid.iScansNegatively = xPlus() ? 0L : 1L;
    info.grid.jScansPositively = yPlus() ? 1L : 0L;
}


void RegularGrid::fillJob(api::MIRJob& job) const {
    // shape of the reference system
    shape_.fillJob(job, grid_->projection().spec());

    // scanningMode
    std::string grid;
    ASSERT(job.get("grid", grid) && !grid.empty());

    grid += ";iScansNegatively=" + std::to_string(x().front() < x().back() ? 0 : 1);
    grid += ";jScansPositively=" + std::to_string(y().front() < y().back() ? 1 : 0);

    job.set("grid", grid);
}


bool RegularGrid::includesNorthPole() const {
    return bbox_.north() == Latitude::NORTH_POLE;
}


bool RegularGrid::includesSouthPole() const {
    return bbox_.south() == Latitude::SOUTH_POLE;
}


bool RegularGrid::isPeriodicWestEast() const {
    return includesNorthPole() || includesSouthPole() ||
           (bbox_.east().value() - bbox_.west().value() >= Longitude::GLOBE.value());
}


void RegularGrid::reorder(long /*scanningMode*/, MIRValuesVector& /*unused*/) const {
    // do not reorder, iterator is doing the right thing
    // FIXME this function should not be overriding to do nothing
}


void RegularGrid::validate(const MIRValuesVector& values) const {
    const size_t count = numberOfPoints();

    Log::debug() << "RegularGrid::validate checked " << Log::Pretty(values.size(), {"value"}) << ", iterator counts "
                 << Log::Pretty(count) << " (" << domain() << ")." << std::endl;

    ASSERT_VALUES_SIZE_EQ_ITERATOR_COUNT("RegularGrid", values.size(), count);
}


Iterator* RegularGrid::iterator() const {
    const auto& [lats, lons] = grid_->to_latlons();
    return new iterator::UnstructuredIterator(lats, lons);
}


void RegularGrid::makeName(std::ostream& out) const {
    eckit::MD5 h;
    h << grid_->uid();
    h << firstPointBottomLeft_;

    if (shape_.provided) {
        h << shape_.code;
        h << shape_.a;
        h << shape_.b;
    }

    out << "RegularGrid-" << grid_->type() << "-" << h.digest();
}


PointXY RegularGrid::firstPointXY() const {
    return {
        firstPointBottomLeft_ ? grid_->x().min() : grid_->x().values().front(),
        firstPointBottomLeft_ ? grid_->y().min() : grid_->y().values().front(),
    };
}


PointLonLat RegularGrid::firstPointLonLat() const {
    return std::get<PointLonLat>(grid_->projection().inv(firstPointXY()));
}


PointLonLat RegularGrid::referencePointLonLat() const {
    return std::get<PointLonLat>(grid_->projection().inv(PointXY{0., 0.}));
}


bool RegularGrid::sameAs(const Representation& other) const {
    auto name = [](const RegularGrid& repres) {
        std::ostringstream str;
        repres.makeName(str);
        return str.str();
    };

    const auto* o = dynamic_cast<const RegularGrid*>(&other);
    return (o != nullptr) && name(*this) == name(*o);
}


void RegularGrid::fillMeshGen(util::MeshGeneratorParameters& params) const {
    if (params.meshGenerator_.empty()) {
        params.meshGenerator_ = "structured";
    }
}


}  // namespace mir::repres::regular
