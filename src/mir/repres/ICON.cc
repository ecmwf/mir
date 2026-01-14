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


#include "mir/repres/ICON.h"

#include <algorithm>
#include <cctype>
#include <ostream>
#include <vector>

#include "eckit/geo/area/BoundingBox.h"
#include "eckit/geo/grid/unstructured/ICON.h"
#include "eckit/types/FloatCompare.h"

#include "mir/api/MIRJob.h"
#include "mir/api/mir_config.h"
#include "mir/iterator/UnstructuredIterator.h"
#include "mir/key/grid/GridPattern.h"
#include "mir/key/grid/NamedGrid.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Iterator.h"
#include "mir/util/BoundingBox.h"
#include "mir/util/Domain.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Grib.h"
#include "mir/util/MeshGeneratorParameters.h"


namespace mir::repres {


namespace {


class NamedICON : public key::grid::NamedGrid {
public:
    // -- Constructors

    explicit NamedICON(const std::string& key) : NamedGrid(key) {}

protected:
    // -- Overridden methods

    void print(std::ostream& out) const override { out << "NamedICON[key=" << key_ << "]"; }
    size_t gaussianNumber() const override { return default_gaussian_number(); }
    const repres::Representation* representation() const override { return new ICON(key_); }
    const repres::Representation* representation(const util::Rotation&) const override { NOTIMP; }
};


const std::string PATTERN("^[iI][cC][oO][nN]-([gG][rR][iI][dD]-(....)-(......)(-(.*))?|[cC][hH].(-[vV][1-9][0-9]*)?)$");


class ICONPattern : public key::grid::GridPattern {
public:
    // -- Constructors

    explicit ICONPattern(const std::string& pattern) : GridPattern(pattern) {}

private:
    // -- Overridden methods

    void print(std::ostream& out) const override { out << "ICONPattern[pattern=" << pattern_ << "]"; }

    const key::grid::Grid* make(const std::string& name) const override { return new NamedICON(name); }

    std::string canonical(const std::string& name, const param::MIRParametrisation& param) const override {
        ASSERT(!name.empty());

        auto can(name);
        std::transform(can.begin(), can.end(), can.begin(),
                       [](unsigned char c) { return c == '_' ? '-' : std::tolower(c); });

        if (can.find("-ch") != std::string::npos &&  //
            can.find("-v") == std::string::npos) {
            can += "-v1";
        }

        return can;
    }
};


const ICONPattern __ICON(PATTERN);


}  // namespace


ICON::ICON(const std::string& grid) :
    ICON([&grid]() {
        eckit::spec::Custom custom{{"grid", grid}};
        std::unique_ptr<eckit::geo::Grid::Spec> spec(eckit::geo::GridFactory::make_spec(custom));
        ASSERT(spec);

        return new grid_type(*spec);
    }()) {}


ICON::ICON(const param::MIRParametrisation& param) :
    ICON([&param]() {
        std::string uid;
        ASSERT(param.get("uid", uid));
        return uid;
    }()) {}


ICON::ICON(grid_type* grid_ptr) :
    Gridded([grid_ptr]() {
        ASSERT(grid_ptr != nullptr);
        auto [n, w, s, e] = grid_ptr->boundingBox().deconstruct();
        return util::BoundingBox{n, w, s, e};
    }()),
    grid_(grid_ptr) {
    ASSERT(grid_);
}


std::string ICON::match(const std::string& name, const param::MIRParametrisation& param) {
    return key::grid::GridPattern::match(name, param);
}


const ICON::points_type& ICON::to_latlons() const {
    if (points_.first.empty() || points_.second.empty()) {
        ASSERT(points_.first.empty() && points_.second.empty());

        points_ = grid_->to_latlons();
        ASSERT(points_.first.size() == points_.second.size());
        ASSERT(points_.first.size() == numberOfPoints());
    }

    return points_;
}


bool ICON::sameAs(const Representation& other) const {
    const auto* o = dynamic_cast<const ICON*>(&other);
    return (o != nullptr) && *grid_ == *(o->grid_);
}


void ICON::makeName(std::ostream& out) const {
    out << grid_->name() << "-" << grid_->arrangement() << "-" << grid_->uid();
}


void ICON::fillGrib(grib_info& info) const {
    info.grid.grid_type        = GRIB_UTIL_GRID_SPEC_UNSTRUCTURED;
    info.packing.editionNumber = 2;

    info.extra_set("uuidOfHGrid", grid_->uid().c_str());

    const auto& catalog = grid_->catalog();
    if (static const std::string key{"icon_number_of_grid_used"}; catalog.has(key)) {
        info.extra_set("numberOfGridUsed", static_cast<long>(catalog.get_unsigned(key)));
    }

    if (static const std::string key{"icon_number_of_grid_in_reference"}; catalog.has(key)) {
        info.extra_set("numberOfGridInReference", static_cast<long>(catalog.get_unsigned(key)));
    }
}


void ICON::fillMeshGen(util::MeshGeneratorParameters& params) const {
    if (params.meshGenerator_.empty()) {
        params.meshGenerator_ = "delaunay";
    }
}


void ICON::fillJob(api::MIRJob& job) const {
    const auto& spec = static_cast<const eckit::geo::Grid&>(*grid_).spec();
    job.set("grid", spec.get_string(spec.has("uid") ? "uid" : "grid"));
}


void ICON::json(eckit::JSON& j) const {
    const auto& spec = static_cast<const eckit::geo::Grid&>(*grid_).spec();
    spec.json(j);
}


Iterator* ICON::iterator() const {
    const auto& [lats, lons] = to_latlons();
    return new iterator::UnstructuredIterator(lats, lons);
}


bool ICON::includesNorthPole() const {
    return eckit::types::is_approximately_equal(bbox_.north().value(), Latitude::NORTH_POLE.value());
}


bool ICON::includesSouthPole() const {
    return eckit::types::is_approximately_equal(bbox_.south().value(), Latitude::SOUTH_POLE.value());
}


bool ICON::isPeriodicWestEast() const {
    return eckit::types::is_approximately_greater_or_equal(bbox_.east().value() - bbox_.west().value(),
                                                           Longitude::GLOBE.value());
}


void ICON::print(std::ostream& out) const {
    out << "ICON[name=" << grid_->name() << ",arrangement=" << grid_->arrangement() << ",uid=" << grid_->uid() << "]";
}


void ICON::validate(const MIRValuesVector& values) const {
    ASSERT_VALUES_SIZE_EQ_ITERATOR_COUNT("ICON", values.size(), numberOfPoints());
}


size_t ICON::numberOfPoints() const {
    return grid_->size();
}


atlas::Grid ICON::atlasGrid() const {
    auto grid = other::UnstructuredGrid::atlas_unstructured_grid_from_points(to_latlons(), domain());
    ASSERT(grid.size() == grid_->size());
    return grid;
}


static const RepresentationBuilder<ICON> __grid("icon");


}  // namespace mir::repres
