// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/param/GridSpecParametrisation.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <ostream>
#include <utility>

#include "eckit/geo/grid/Reduced.h"
#include "eckit/geo/grid/reduced/HEALPix.h"
#include "eckit/geo/grid/reduced/ReducedGaussian.h"
#include "eckit/geo/grid/reduced/ReducedLonLat.h"
#include "eckit/geo/grid/regular/RegularGaussian.h"
#include "eckit/geo/projection/Rotation.h"

#include "mir/util/Exceptions.h"


namespace mir::param {


namespace {


using eckit::geo::Grid;
using eckit::geo::grid::Reduced;
using eckit::geo::grid::reduced::HEALPix;
using eckit::geo::grid::reduced::ReducedGaussian;
using eckit::geo::grid::reduced::ReducedLonLat;
using eckit::geo::grid::regular::RegularGaussian;
using eckit::geo::projection::Rotation;


const Rotation* rotation(const Grid& grid) {
    const auto* r = dynamic_cast<const Rotation*>(&grid.projection());
    return r != nullptr && r->rotated() ? r : nullptr;
}


std::string grib_grid_type(const Grid& grid) {
    // GRIB gridType (unrotated, rotated), other grids are represented by their gridspec
    static const std::map<std::string, std::pair<std::string, std::string>> types{
        {"regular_ll", {"regular_ll", "rotated_ll"}},
        {"regular_gg", {"regular_gg", "rotated_gg"}},
        {"reduced_gg", {"reduced_gg", "reduced_rotated_gg"}},
        {"HEALPix", {"healpix", ""}},
        {"unstructured_ll", {"unstructured_ll", ""}},
        {"sh", {"sh", "rotated_sh"}},
        {"regular_xy", {"regular_xy", ""}},
    };

    auto it = types.find(grid.type());
    if (it == types.end()) {
        return "unstructured_grid";
    }

    if (rotation(grid) == nullptr) {
        return it->second.first;
    }

    if (it->second.second.empty()) {
        throw exception::UserError("GridSpecParametrisation: unsupported rotated grid type: '" + grid.type() + "'");
    }

    return it->second.second;
}


std::string grid_name(const Grid& grid) {
    std::string name;
    grid.spec().get("grid", name);
    return name;
}


}  // namespace


GridSpecParametrisation::GridSpecParametrisation(const std::string& gridspec) :
    GridSpecParametrisation(eckit::geo::GridFactory::make_from_string(gridspec)) {}


GridSpecParametrisation::GridSpecParametrisation(const eckit::geo::Grid* grid) : grid_(grid) {
    ASSERT(grid_);
}


const eckit::geo::Grid& GridSpecParametrisation::grid() const {
    ASSERT(grid_);
    return *grid_;
}


const eckit::spec::Spec& GridSpecParametrisation::spec() const {
    return grid().spec();
}


bool GridSpecParametrisation::cached(const std::string& name) const {
    if (cache_.has(name)) {
        return true;
    }

    auto set = [this, &name](const auto& value) {
        cache_.set(name, value);
        return true;
    };

    const auto& grid    = *grid_;
    const auto& type    = grid.type();
    const auto gridType = grib_grid_type(grid);

    const auto is_regular = type == "regular_ll" || type == "regular_gg";
    const auto* healpix   = dynamic_cast<const HEALPix*>(&grid);
    const auto* reduced   = healpix == nullptr ? dynamic_cast<const Reduced*>(&grid) : nullptr;

    // grid type
    if (name == "gridType") {
        return set(gridType);
    }

    if (name == "gridded") {
        return type != "sh" && set(true);
    }

    if (name == "spectral") {
        return type == "sh" && set(true);
    }

    if (name == "gridded_regular_ll") {
        return type == "regular_ll" && set(true);
    }

    // grid name
    if (name == "grid" && type == "regular_ll") {
        return set(std::vector<double>{std::abs(grid.dx()), std::abs(grid.dy())});
    }

    if (name == "grid" && healpix != nullptr) {
        return set((healpix->order() == "nested" ? "HN" : "H") + std::to_string(healpix->Nside()));
    }

    if ((name == "grid" || name == "gridName") && type != "regular_ll" && type != "sh") {
        auto value = healpix != nullptr ? "H" + std::to_string(healpix->Nside()) : grid_name(grid);
        return !value.empty() && set(value);
    }

    // points ordering
    if (name == "order" && (is_regular || reduced != nullptr || healpix != nullptr)) {
        return set(grid.order());
    }

    // regular grids
    if (is_regular) {
        if (name == "Ni") {
            return set(static_cast<long>(grid.nx()));
        }

        if (name == "Nj") {
            return set(static_cast<long>(grid.ny()));
        }

        if (name == "north" || name == "south") {
            return set(name == "north" ? grid.y().max() : grid.y().min());
        }

        if (name == "west" || name == "east") {
            return set(name == "west" ? grid.x().min() : grid.x().max());
        }

        if (name == "west_east_increment") {
            return set(std::abs(grid.dx()));
        }

        if (name == "south_north_increment" && type == "regular_ll") {
            return set(std::abs(grid.dy()));
        }
    }

    // reduced grids (a regional grid keeps all latitudes, those without points have no longitudes)
    if (reduced != nullptr &&
        (name == "Nj" || name == "north" || name == "south" || name == "west" || name == "east")) {
        const auto& lats = reduced->latitudes();

        long nj    = 0;
        auto north = lats.front();
        auto south = lats.front();
        auto west  = 0.;
        auto east  = 0.;

        for (size_t j = 0; j < lats.size(); ++j) {
            if (const auto& lons = reduced->longitudes(j); !lons.empty()) {
                north = nj == 0 ? lats[j] : std::max(north, lats[j]);
                south = nj == 0 ? lats[j] : std::min(south, lats[j]);
                west  = nj == 0 ? lons.front() : std::min(west, lons.front());
                east  = nj == 0 ? lons.back() : std::max(east, lons.back());
                ++nj;
            }
        }

        cache_.set("Nj", nj);
        cache_.set("north", north);
        cache_.set("south", south);
        cache_.set("west", west);
        cache_.set("east", east);
        return true;
    }

    if (name == "pl") {
        if (const auto* g = dynamic_cast<const ReducedGaussian*>(&grid); g != nullptr) {
            return set(g->pl());
        }

        if (const auto* g = dynamic_cast<const ReducedLonLat*>(&grid); g != nullptr) {
            return set(g->pl());
        }
    }

    // HEALPix
    if (healpix != nullptr) {
        if (name == "Nside") {
            return set(static_cast<long>(healpix->Nside()));
        }

        if (name == "orderingConvention") {
            return set(healpix->order());
        }

        if (name == "longitudeOfFirstGridPointInDegrees" || name == "west") {
            return set(45.);
        }
    }

    // Gaussian grids
    if (const auto* g = dynamic_cast<const RegularGaussian*>(&grid); g != nullptr) {
        if (name == "N" || name == "regular") {
            return set(static_cast<long>(g->N()));
        }
    }

    if (const auto* g = dynamic_cast<const ReducedGaussian*>(&grid); g != nullptr) {
        if (name == "N" || name == (g->octahedral() ? "octahedral" : "reduced")) {
            return set(static_cast<long>(g->N()));
        }
    }

    // rotation
    if (const auto* r = rotation(grid); r != nullptr) {
        if (name == "south_pole_latitude") {
            return set(r->south_pole().lat());
        }

        if (name == "south_pole_longitude") {
            return set(r->south_pole().lon());
        }

        if (name == "south_pole_rotation_angle") {
            return set(r->angle());
        }

        if (name == "rotation") {
            return set(std::vector<double>{r->south_pole().lat(), r->south_pole().lon()});
        }
    }

    // spectral
    if (name == "truncation" && type == "sh") {
        return set(static_cast<long>(grid.truncation()));
    }

    // unstructured grids
    if (name == "uid" && (gridType == "unstructured_grid" || gridType == "unstructured_ll")) {
        return set(grid.uid());
    }

    if (name == "gridspec" && (gridType == "unstructured_grid" || gridType == "regular_xy")) {
        return set(grid.spec_str());
    }

    if ((name == "latitudes" || name == "longitudes") && gridType == "unstructured_ll") {
        auto [lats, lons] = grid.to_latlons();
        cache_.set("latitudes", lats);
        cache_.set("longitudes", lons);
        return true;
    }

    return false;
}


bool GridSpecParametrisation::has(const std::string& name) const {
    util::lock_guard<util::recursive_mutex> lock(mutex_);

    if (static const std::string LL{"latitudes/longitudes"}; name == LL) {
        return cached("latitudes") && cached("longitudes");
    }

    return cached(name);
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
    out << "GridSpecParametrisation[" << grid_->spec_str() << "]";
}


}  // namespace mir::param
