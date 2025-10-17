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

#include <ostream>

#include "eckit/geo/area/BoundingBox.h"
#include "eckit/geo/grid/HEALPix.h"
#include "eckit/geo/grid/ReducedGaussian.h"
#include "eckit/geo/grid/RegularGaussian.h"
#include "eckit/geo/grid/RegularLL.h"
#include "eckit/geo/projection/Rotation.h"

#include "mir/util/Exceptions.h"


namespace mir::param {


namespace {


struct GridMapping : GridSpecParametrisation::Mapping {};


struct AreaMapping : GridSpecParametrisation::Mapping {};


struct ProjectionMapping : GridSpecParametrisation::Mapping {};


struct MappingGridRegularLL : GridMapping {
    explicit MappingGridRegularLL(const eckit::geo::Grid& _grid) :
        grid_(dynamic_cast<const eckit::geo::grid::RegularLL&>(_grid)) {}

    void fill(SimpleParametrisation& param) const override {
        param.set("gridType", "regular_ll");
        param.set("gridded", 1L);

        param.set("west_east_increment", grid_.dlon());
        param.set("south_north_increment", grid_.dlat());
        param.set("grid", std::vector<double>{grid_.dlon(), grid_.dlat()});

        param.set("Ni", grid_.nlon());
        param.set("Nj", grid_.nlat());
    }

    const eckit::geo::grid::RegularLL& grid_;
};


struct MappingGridRegularGG : GridMapping {
    explicit MappingGridRegularGG(const eckit::geo::Grid& _grid) :
        grid_(dynamic_cast<const eckit::geo::grid::RegularGaussian&>(_grid)) {}

    void fill(SimpleParametrisation& param) const override {
        param.set("gridType", "regular_gg");
        param.set("gridded", 1L);
        param.set("N", grid_.N());
    }

    const eckit::geo::grid::RegularGaussian& grid_;
};


struct MappingGridHEALPix : GridMapping {
    explicit MappingGridHEALPix(const eckit::geo::Grid& _grid) :
        grid_(dynamic_cast<const eckit::geo::grid::HEALPix&>(_grid)) {}

    void fill(SimpleParametrisation& param) const override {
        param.set("gridType", "healpix");
        param.set("gridded", 1L);
        param.set("Nside", grid_.Nside());
        param.set("orderingConvention", grid_.order());
        param.set("longitudeOfFirstGridPointInDegrees", 45.);
    }

    const eckit::geo::grid::HEALPix& grid_;
};


struct MappingGridByUID : GridMapping {
    MappingGridByUID(const std::string& type, const eckit::geo::Grid::uid_type& uid) : type_(type), uid_(uid) {}

    void fill(SimpleParametrisation& param) const override {
        param.set("gridType", type_);
        param.set("gridded", 1L);
        param.set("uid", uid_);
    }

    const std::string type_;
    const eckit::geo::Grid::uid_type uid_;
};


struct MappingGridReducedGG : GridMapping {
    explicit MappingGridReducedGG(const eckit::geo::Grid& _grid) :
        grid_(dynamic_cast<const eckit::geo::grid::ReducedGaussian&>(_grid)) {}

    void fill(SimpleParametrisation& param) const override {
        param.set("gridType", "reduced_gg");
        param.set("gridded", 1L);
        param.set("N", grid_.N());
    }

    const eckit::geo::grid::ReducedGaussian& grid_;
};


struct MappingAreaBoundingBox : AreaMapping {
    explicit MappingAreaBoundingBox(const eckit::geo::Area& _area) :
        area(dynamic_cast<const eckit::geo::area::BoundingBox&>(_area)) {}

    void fill(SimpleParametrisation& param) const override {
        param.set("north", area.north);
        param.set("west", area.west);
        param.set("south", area.south);
        param.set("east", area.east);
    }

    const eckit::geo::area::BoundingBox& area;
};


struct MappingAreaNone : AreaMapping {
    explicit MappingAreaNone(const eckit::geo::Area&) {}
};


struct MappingProjectionRotation : ProjectionMapping {
    explicit MappingProjectionRotation(const eckit::geo::Projection& _proj) :
        projection(dynamic_cast<const eckit::geo::projection::Rotation&>(_proj)) {}

    void fill(SimpleParametrisation& param) const override { NOTIMP; }

    const eckit::geo::projection::Rotation& projection;
};


struct MappingProjectionNone : ProjectionMapping {
    explicit MappingProjectionNone(const eckit::geo::Projection&) {}
};


GridMapping* build_grid_mapping(const eckit::geo::Grid& grid) {
    const auto& type = grid.type();

    return type == "regular-ll"   ? static_cast<GridMapping*>(new MappingGridRegularLL(grid))
           : type == "regular-gg" ? static_cast<GridMapping*>(new MappingGridRegularGG(grid))
           : type == "reduced-gg" ? static_cast<GridMapping*>(new MappingGridReducedGG(grid))
           : type == "healpix"    ? static_cast<GridMapping*>(new MappingGridHEALPix(grid))
           : type == "fesom" || type == "icon" || type == "orca"
               ? static_cast<GridMapping*>(new MappingGridByUID(type, grid.uid()))
               : throw exception::UserError("GridSpecParametrisation: unsupported grid mapping type: '" + type + "'");
}


AreaMapping* build_area_mapping(const eckit::geo::Area& area) {
    const auto& type = area.type();
    return type == "bounding-box" ? static_cast<AreaMapping*>(new MappingAreaBoundingBox(area))
           : type == "none"
               ? static_cast<AreaMapping*>(new MappingAreaNone(area))
               : throw exception::UserError("GridSpecParametrisation: unsupported area mapping type: '" + type + "'");
}


ProjectionMapping* build_projection_mapping(const eckit::geo::Projection& projection) {
    const auto& type = projection.type();
    return type == "rotation" ? static_cast<ProjectionMapping*>(new MappingProjectionRotation(projection))
           : type == "none"   ? static_cast<ProjectionMapping*>(new MappingProjectionNone(projection))
           : type == "eqc"    ? static_cast<ProjectionMapping*>(new MappingProjectionNone(projection))
                              : throw exception::UserError(
                                 "GridSpecParametrisation: unsupported projection mapping type: '" + type + "'");
}


struct Mappings : GridSpecParametrisation::Mapping {
    explicit Mappings(const eckit::geo::Grid& _grid) :
        grid(build_grid_mapping(_grid)),
        area(build_area_mapping(_grid.area())),
        projection(build_projection_mapping(_grid.projection())) {
        ASSERT(grid);
        ASSERT(area);
        ASSERT(projection);
    }

    void fill(SimpleParametrisation& param) const override {
        grid->fill(param);
        area->fill(param);
        projection->fill(param);
    }

    std::unique_ptr<GridMapping> grid;
    std::unique_ptr<AreaMapping> area;
    std::unique_ptr<ProjectionMapping> projection;
};


}  // namespace


GridSpecParametrisation::GridSpecParametrisation(const std::string& gridspec) :
    GridSpecParametrisation(eckit::geo::GridFactory::make_from_string(gridspec)) {}


GridSpecParametrisation::GridSpecParametrisation(const eckit::geo::Grid* grid) :
    grid_(grid), spec_(grid_->spec()), mapping_(new Mappings(*grid_)) {
    ASSERT(mapping_);
    mapping_->fill(cache_);

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
        if (grid_->type() == "reduced-gg") {
            const auto& g = dynamic_cast<const eckit::geo::grid::ReducedGaussian&>(*grid_);

            value = g.pl();
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
