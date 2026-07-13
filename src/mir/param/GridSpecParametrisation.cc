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


#include "mir/param/GridSpecParametrisation.h"

#include <cmath>
#include <ostream>

#include "eckit/geo/area/BoundingBox.h"
#include "eckit/geo/grid/SphericalHarmonics.h"
#include "eckit/geo/grid/reduced/HEALPix.h"
#include "eckit/geo/grid/reduced/ReducedGaussian.h"
#include "eckit/geo/grid/reduced/ReducedLonLat.h"
#include "eckit/geo/grid/regular/RegularGaussian.h"
#include "eckit/geo/grid/regular/RegularLL.h"
#include "eckit/geo/projection/Rotation.h"

#include "mir/util/Exceptions.h"


namespace mir::param {


namespace {


void fill_scanning_mode(SimpleParametrisation& param, const eckit::geo::Grid& grid) {
    param.set("order", grid.order());
}


void fill_grid(SimpleParametrisation& param, const eckit::geo::Grid& grid) {
    const auto& type = grid.type();

    if (type == "regular_ll") {
        const auto& g = dynamic_cast<const eckit::geo::grid::regular::RegularLL&>(grid);

        param.set("gridType", "regular_ll");
        param.set("gridded", true);

        auto dlon = g.dx();
        const std::string i(dlon < 0. ? "i-" : "i+");
        param.set("west_east_increment", std::abs(dlon));

        auto dlat = g.dy();
        const std::string j(dlat < 0. ? "j-" : "j+");
        param.set("south_north_increment", std::abs(dlat));

        param.set("grid", std::vector<double>{dlon, dlat});
        param.set("order", i + j);

        param.set("Ni", g.nx());
        param.set("Nj", g.ny());

        fill_scanning_mode(param, grid);
        return;
    }

    if (type == "regular_gg") {
        const auto& g = dynamic_cast<const eckit::geo::grid::regular::RegularGaussian&>(grid);
        param.set("gridType", "regular_gg");
        param.set("gridded", true);
        param.set("N", g.N());

        fill_scanning_mode(param, grid);
        return;
    }

    if (type == "reduced_gg") {
        const auto& g = dynamic_cast<const eckit::geo::grid::reduced::ReducedGaussian&>(grid);
        param.set("gridType", "reduced_gg");
        param.set("gridded", true);
        param.set("N", g.N());
        param.set("pl", g.pl());

        fill_scanning_mode(param, grid);
        return;
    }

    if (type == "HEALPix") {
        const auto& g = dynamic_cast<const eckit::geo::grid::reduced::HEALPix&>(grid);
        param.set("grid", (g.order() == "nested" ? "HN" : "H") + std::to_string(g.Nside()));
        param.set("gridType", "healpix");
        param.set("gridded", true);
        param.set("Nside", g.Nside());
        param.set("orderingConvention", g.order());
        param.set("longitudeOfFirstGridPointInDegrees", 45.);
        return;
    }

    if (type == "FESOM" || type == "ICON" || type == "ORCA" || type == "unstructured_ll") {
        param.set("gridType", type);
        param.set("gridded", true);
        param.set("uid", grid.uid());

        if (type == "unstructured_ll") {
            auto [lats, lons] = grid.to_latlons();
            param.set("latitudes", lats);
            param.set("longitudes", lons);
        }

        return;
    }

    if (type == "sh") {
        const auto& g = dynamic_cast<const eckit::geo::grid::SphericalHarmonics&>(grid);
        param.set("gridType", "sh");
        param.set("spectral", true);
        param.set("truncation", g.truncation());
        return;
    }

    throw exception::UserError("GridSpecParametrisation: unsupported grid type: '" + type + "'");
}


void fill_area(SimpleParametrisation& param, const eckit::geo::Grid& grid) {
    const auto& area = grid.area();
    const auto& type = area.type();

    if (type == "bounding_box") {
        if (auto grid_type = grid.type(); grid_type == "regular_ll") {
            const auto& g = dynamic_cast<const eckit::geo::grid::regular::RegularLL&>(grid);
            param.set("north", g.y().max());
            param.set("west", g.x().min());
            param.set("south", g.y().min());
            param.set("east", g.x().max());
        }
        else {
            const auto& a = dynamic_cast<const eckit::geo::area::BoundingBox&>(area);
            param.set("north", a.north());
            param.set("west", a.west());
            param.set("south", a.south());
            param.set("east", a.east());
        }
        return;
    }

    if (type == "none") {
        return;
    }

    throw exception::UserError("GridSpecParametrisation: unsupported area type: '" + type + "'");
}


void fill_projection(SimpleParametrisation& param, const eckit::geo::Projection& projection) {
    const auto& type = projection.type();

    if (type == "rotation") {
        static_cast<void>(param);
        NOTIMP;
    }

    if (type == "none" || type == "eqc") {
        return;
    }

    throw exception::UserError("GridSpecParametrisation: unsupported projection type: '" + type + "'");
}


}  // namespace


GridSpecParametrisation::GridSpecParametrisation(const std::string& gridspec) :
    GridSpecParametrisation(eckit::geo::GridFactory::make_from_string(gridspec)) {}


GridSpecParametrisation::GridSpecParametrisation(const eckit::geo::Grid* grid) : grid_(grid), spec_(grid_->spec()) {
    fill_grid(cache_, *grid);
    fill_area(cache_, *grid);
    fill_projection(cache_, grid->projection());

    ASSERT(has("gridType"));
}


const eckit::geo::Grid& GridSpecParametrisation::grid() const {
    ASSERT(grid_);
    return *grid_;
}


bool GridSpecParametrisation::has(const std::string& name) const {
    util::lock_guard<util::recursive_mutex> lock(mutex_);
    return cache_.has(name) || spec_.has(name);
}


bool GridSpecParametrisation::get(const std::string& name, std::string& value) const {
    return _get(name, value);
}


bool GridSpecParametrisation::get(const std::string& name, bool& value) const {
    return _get(name, value);
}


bool GridSpecParametrisation::get(const std::string& name, int& value) const {
    return _get(name, value);
}


bool GridSpecParametrisation::get(const std::string& name, long& value) const {
    return _get(name, value);
}


bool GridSpecParametrisation::get(const std::string& name, float& value) const {
    return _get(name, value);
}


bool GridSpecParametrisation::get(const std::string& name, double& value) const {
    return _get(name, value);
}


bool GridSpecParametrisation::get(const std::string& name, std::vector<int>& value) const {
    return _get(name, value);
}


bool GridSpecParametrisation::get(const std::string& name, std::vector<long>& value) const {
    if (name == "pl") {
        using eckit::geo::grid::reduced::ReducedGaussian;
        if (const auto* g = dynamic_cast<const ReducedGaussian*>(grid_.get()); g != nullptr) {
            value = g->pl();
            return true;
        }

        using eckit::geo::grid::reduced::ReducedLonLat;
        if (const auto* g = dynamic_cast<const ReducedLonLat*>(grid_.get()); g != nullptr) {
            value = g->pl();
            return true;
        }
    }

    return _get(name, value);
}


bool GridSpecParametrisation::get(const std::string& name, std::vector<float>& value) const {
    return _get(name, value);
}


bool GridSpecParametrisation::get(const std::string& name, std::vector<double>& value) const {
    return _get(name, value);
}


bool GridSpecParametrisation::get(const std::string& name, std::vector<std::string>& value) const {
    return _get(name, value);
}


void GridSpecParametrisation::print(std::ostream& out) const {
    out << "GridSpecParametrisation[cache=" << cache_ << "]";
}


}  // namespace mir::param
