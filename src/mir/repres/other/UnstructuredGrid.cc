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


#include "mir/repres/other/UnstructuredGrid.h"

#include <cctype>
#include <limits>
#include <memory>
#include <numeric>
#include <ostream>
#include <string>
#include <utility>

#include "eckit/filesystem/PathName.h"
#include "eckit/parser/YAMLParser.h"
#include "eckit/types/FloatCompare.h"
#include "eckit/utils/MD5.h"

#include "mir/api/MIRJob.h"
#include "mir/api/mir_config.h"
#include "mir/input/GriddefInput.h"
#include "mir/iterator/UnstructuredIterator.h"
#include "mir/output/GriddefOutput.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/other/UnstructuredGrid.h"
#include "mir/util/Atlas.h"
#include "mir/util/BoundingBox.h"
#include "mir/util/CheckDuplicatePoints.h"
#include "mir/util/Domain.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Grib.h"
#include "mir/util/Log.h"
#include "mir/util/MeshGeneratorParameters.h"
#include "mir/util/ValueMap.h"

#if mir_HAVE_ECKIT_CODEC
#include <algorithm>

#include "eckit/geo/area/BoundingBox.h"
#include "eckit/geo/grid/ORCA.h"
#include "eckit/geo/grid/unstructured/FESOM.h"
#include "eckit/geo/grid/unstructured/ICON.h"
#include "eckit/spec/Custom.h"
#include "eckit/spec/Spec.h"

#include "mir/key/grid/GridPattern.h"
#include "mir/key/grid/NamedGrid.h"
#endif


namespace mir::repres {


#if mir_HAVE_ECKIT_CODEC
namespace {


class UnstructuredGridFromUID : public Gridded {
public:
    explicit UnstructuredGridFromUID(const std::string& grid) :
        UnstructuredGridFromUID([&grid]() {
            eckit::spec::Custom custom{{"grid", grid}};
            std::unique_ptr<eckit::geo::Grid::Spec> spec(eckit::geo::GridFactory::make_spec(custom));
            return eckit::geo::GridFactory::build(*spec);
        }()) {}

    explicit UnstructuredGridFromUID(const param::MIRParametrisation& param) :
        UnstructuredGridFromUID([&param]() {
            std::string uid;
            ASSERT(param.get("uid", uid));
            return uid;
        }()) {}

protected:
    using points_type = std::pair<std::vector<double>, std::vector<double>>;
    using grid_type   = const eckit::geo::Grid;

    explicit UnstructuredGridFromUID(grid_type* grid_ptr) :
        Gridded([grid_ptr]() {
            ASSERT(grid_ptr != nullptr);
            auto [n, w, s, e] = grid_ptr->boundingBox().deconstruct();
            return util::BoundingBox{n, w, s, e};
        }()),
        grid_(grid_ptr) {
        ASSERT(grid_);
    }

    const points_type& to_latlons() const {
        if (points_.first.empty() || points_.second.empty()) {
            ASSERT(points_.first.empty() && points_.second.empty());

            points_ = grid_->to_latlons();
            ASSERT(points_.first.size() == points_.second.size());
            ASSERT(points_.first.size() == numberOfPoints());
        }

        return points_;
    }

    bool sameAs(const Representation& other) const override {
        const auto* o = dynamic_cast<const UnstructuredGridFromUID*>(&other);
        return (o != nullptr) && *grid_ == *(o->grid_);
    }

    void makeName(std::ostream& out) const override {
        out << grid_->type() << '-' << numberOfPoints() << '-' << grid_->uid();
    }

    void fillMeshGen(util::MeshGeneratorParameters& params) const override {
        if (params.meshGenerator_.empty()) {
            if (const auto uid = grid_ptr()->uid(); atlas::grid::SpecRegistry::has(uid)) {
                auto type = atlas::grid::SpecRegistry::get(uid).getString("type", "");
                std::transform(type.begin(), type.end(), type.begin(),
                               [](auto c) { return c == '_' ? '-' : std::tolower(c); });

                if (atlas::meshgenerator::MeshGeneratorFactory::has(type)) {
                    Log::debug() << "UnstructuredGrid: atlas::MeshGenerator(\"" << type << "\")" << std::endl;
                    params.meshGenerator_ = type;
                    return;
                }
            }

            params.meshGenerator_ = "delaunay";
        }
    }

    void fillJob(api::MIRJob& job) const override {
        util::ValueMap map(eckit::YAMLParser::decodeString(grid_->spec().str()));
        map.set(static_cast<param::SimpleParametrisation&>(job));
    }

    void print(std::ostream& out) const override { out << grid_->spec().str(); }
    void json(eckit::JSON& j) const override { grid_->spec().json(j); }

    size_t numberOfPoints() const override { return grid_->size(); }

    Iterator* iterator() const override {
        const auto& [lats, lons] = to_latlons();
        return new iterator::UnstructuredIterator(lats, lons);
    }

    bool extendBoundingBoxOnIntersect() const override { return false; }

    bool includesNorthPole() const override { return bbox_.north() == Latitude::NORTH_POLE; }
    bool includesSouthPole() const override { return bbox_.south() == Latitude::SOUTH_POLE; }
    bool isPeriodicWestEast() const override {
        return eckit::types::is_approximately_greater_or_equal(bbox_.east().value() - bbox_.west().value(),
                                                               Longitude::GLOBE.value());
    }

    ::atlas::Grid atlasGrid() const override {
        if (const auto uid = grid_->uid(); atlas::grid::SpecRegistry::has(uid)) {
            Log::debug() << "UnstructuredGrid: atlas::Grid(uid=\"" << uid << "\")" << std::endl;
            return {atlas::grid::SpecRegistry::get(uid)};
        }

        const auto& [lats, lons] = to_latlons();
        ASSERT(!lats.empty());
        ASSERT(lats.size() == lons.size());

        const auto N = lats.size();
        std::vector<atlas::PointXY> points(N);
        for (size_t i = 0; i < N; ++i) {
            points[i].assign(lons[i], lats[i]);
        }

        const auto grid = atlas::UnstructuredGrid(std::move(points));
        ASSERT(grid.size() == grid_->size());

        const auto dom = domain();
        return dom.isGlobal() ? grid : atlas::UnstructuredGrid(grid, dom);
    }

    grid_type* grid_ptr() const { return grid_.get(); }

private:
    std::unique_ptr<grid_type> grid_;
    mutable points_type points_;
};


class FESOM final : public UnstructuredGridFromUID {
public:
    using UnstructuredGridFromUID::UnstructuredGridFromUID;

    void fillGrib(grib_info& info) const override {
        info.grid.grid_type        = GRIB_UTIL_GRID_SPEC_UNSTRUCTURED;
        info.packing.editionNumber = 2;

        const auto* ptr = dynamic_cast<const eckit::geo::grid::unstructured::FESOM*>(grid_ptr());
        ASSERT(ptr != nullptr);

        info.extra_set("unstructuredGridType", ptr->name().c_str());
        info.extra_set("unstructuredGridSubtype", ptr->arrangement().c_str());
        info.extra_set("uuidOfHGrid", ptr->uid().c_str());
    }

    void validate(const MIRValuesVector& values) const override {
        ASSERT_VALUES_SIZE_EQ_ITERATOR_COUNT("FESOM", values.size(), numberOfPoints());
    }

    static std::string match(const std::string& name, const param::MIRParametrisation& param) {
        return key::grid::GridPattern::match(name, param);
    }
};


class ICON final : public UnstructuredGridFromUID {
public:
    using UnstructuredGridFromUID::UnstructuredGridFromUID;

    void fillGrib(grib_info& info) const override {
        info.grid.grid_type        = GRIB_UTIL_GRID_SPEC_UNSTRUCTURED;
        info.packing.editionNumber = 2;

        const auto* ptr = dynamic_cast<const eckit::geo::grid::unstructured::ICON*>(grid_ptr());
        ASSERT(ptr != nullptr);

        info.extra_set("uuidOfHGrid", ptr->uid().c_str());

        const auto& catalog = ptr->catalog();
        if (static const std::string key{"icon_number_of_grid_used"}; catalog.has(key)) {
            info.extra_set("numberOfGridUsed", static_cast<long>(catalog.get_unsigned(key)));
        }

        if (static const std::string key{"icon_number_of_grid_in_reference"}; catalog.has(key)) {
            info.extra_set("numberOfGridInReference", static_cast<long>(catalog.get_unsigned(key)));
        }
    }

    void validate(const MIRValuesVector& values) const override {
        ASSERT_VALUES_SIZE_EQ_ITERATOR_COUNT("ICON", values.size(), numberOfPoints());
    }

    static std::string match(const std::string& name, const param::MIRParametrisation& param) {
        return key::grid::GridPattern::match(name, param);
    }
};


class ORCA final : public UnstructuredGridFromUID {
public:
    using UnstructuredGridFromUID::UnstructuredGridFromUID;

    void fillGrib(grib_info& info) const override {
        info.grid.grid_type        = GRIB_UTIL_GRID_SPEC_UNSTRUCTURED;
        info.packing.editionNumber = 2;

        const auto* ptr = dynamic_cast<const eckit::geo::grid::ORCA*>(grid_ptr());
        ASSERT(ptr != nullptr);

        info.extra_set("unstructuredGridType", ptr->name().c_str());
        info.extra_set("unstructuredGridSubtype", ptr->arrangement().c_str());
        info.extra_set("uuidOfHGrid", ptr->uid().c_str());
    }

    void validate(const MIRValuesVector& values) const override {
        ASSERT_VALUES_SIZE_EQ_ITERATOR_COUNT("ORCA", values.size(), numberOfPoints());
    }

    static std::string match(const std::string& name, const param::MIRParametrisation& param) {
        return key::grid::GridPattern::match(name, param);
    }
};


class NamedFESOM : public key::grid::NamedGrid {
public:
    // -- Constructors

    explicit NamedFESOM(const std::string& key) : NamedGrid(key) {}

protected:
    // -- Overridden methods

    void print(std::ostream& out) const override { out << "NamedFESOM[key=" << key_ << "]"; }
    size_t gaussianNumber() const override { return default_gaussian_number(); }
    const repres::Representation* representation() const override { return new FESOM(key_); }
    const repres::Representation* representation(const util::Rotation&) const override { NOTIMP; }
};


class FESOMPattern : public key::grid::GridPattern {
public:
    // -- Constructors

    explicit FESOMPattern(const std::string& pattern) : GridPattern(pattern) {}

private:
    // -- Overridden methods

    void print(std::ostream& out) const override { out << "FESOMPattern[pattern=" << pattern_ << "]"; }

    const key::grid::Grid* make(const std::string& name) const override { return new NamedFESOM(name); }

    std::string canonical(const std::string& name, const param::MIRParametrisation& param) const override {
        ASSERT(name.size() >= 2);

        auto can(name);
        std::transform(can.begin(), can.end(), can.begin(), [](auto c) { return std::toupper(c); });

        if (can.find("PI") == 0) {
            can[0] = 'p';
            can[1] = 'i';
        }

        if (can.find('_') == std::string::npos) {
            can += "_C";  // arbitrary choice (to review)
        }

        return can;
    }
};


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
        std::transform(can.begin(), can.end(), can.begin(), [](auto c) { return c == '_' ? '-' : std::tolower(c); });

        if (can.find("-ch") != std::string::npos &&  //
            can.find("-v") == std::string::npos) {
            can += "-v1";
        }

        return can;
    }
};


class NamedORCA : public key::grid::NamedGrid {
public:
    // -- Constructors

    explicit NamedORCA(const std::string& key) : NamedGrid(key) {}

protected:
    // -- Overridden methods

    void print(std::ostream& out) const override { out << "NamedORCA[key=" << key_ << "]"; }
    size_t gaussianNumber() const override { return default_gaussian_number(); }
    const repres::Representation* representation() const override { return new ORCA(key_); }
    const repres::Representation* representation(const util::Rotation&) const override { NOTIMP; }
};


const std::string ORCA_PATTERN("^([eE])?[oO][rR][cC][aA]([0-9]+)(_[tTuUvVwWfF])?$");


class ORCAPattern : public key::grid::GridPattern {
public:
    // -- Constructors

    explicit ORCAPattern(const std::string& pattern) : GridPattern(pattern) {}

private:
    // -- Overridden methods

    void print(std::ostream& out) const override { out << "ORCAPattern[pattern=" << pattern_ << "]"; }

    const key::grid::Grid* make(const std::string& name) const override { return new NamedORCA(name); }

    std::string canonical(const std::string& name, const param::MIRParametrisation& param) const override {
        ASSERT(!name.empty());

        static const std::regex rex(ORCA_PATTERN);

        std::smatch match;
        ASSERT(std::regex_search(name, match, rex) && match.size() == 4);

        auto e(match[1].str());
        auto n(match[2].str());
        auto a(match[3].str());

        if (e.size() == 1) {
            e = static_cast<char>(std::tolower(e.back()));
        }

        if (a.empty()) {
            a = "T";  // arbitrary choice (to review)
            param.get("orca-arrangement", a);
        }
        else if (a.size() == 2) {
            a = static_cast<char>(std::toupper(a.back()));
        }
        ASSERT(a.size() == 1);

        return e + "ORCA" + n + "_" + a;
    }
};


const FESOMPattern __FESOM("^([cC][oO][rR][eE]2|[dD][aA][rR][tT]|[nN][gG]5|[pP][iI])(_[cCnN])?$");
const ICONPattern __ICON("^[iI][cC][oO][nN]-([gG][rR][iI][dD]-(....)-(......)(-(.*))?|[cC][hH].(-[vV][1-9][0-9]*)?)$");
const ORCAPattern __ORCA(ORCA_PATTERN);

const RepresentationBuilder<FESOM> REPRESENTATION_1("fesom");
const RepresentationBuilder<FESOM> REPRESENTATION_2("FESOM");
const RepresentationBuilder<ICON> REPRESENTATION_3("icon");
const RepresentationBuilder<ICON> REPRESENTATION_4("ICON");
const RepresentationBuilder<ORCA> REPRESENTATION_5("orca");
const RepresentationBuilder<ORCA> REPRESENTATION_6("ORCA");


}  // namespace
#endif


template <>
Representation* RepresentationBuilder<other::UnstructuredGrid>::make(const param::MIRParametrisation& param) {
    // NOTE: for grids defined by lat/lon auxiliary data
    if (param.has("latitudes/longitudes")) {
        return new other::UnstructuredGrid(param);
    }

    // specially-named grids
#if mir_HAVE_ECKIT_CODEC
    if (std::string uid; param.get("uid", uid) && eckit::geo::GridSpecByUID::instance().exists(uid)) {
        std::unique_ptr<eckit::spec::Spec> spec(eckit::geo::GridSpecByUID::instance().get(uid).spec());
        ASSERT(spec);

        auto type = spec->get_string("type");
        if (type == "FESOM") {
            return new FESOM(param);
        }

        if (type == "ICON") {
            return new ICON(param);
        }

        if (type == "ORCA") {
            return new ORCA(param);
        }
    }

    if (std::string grid; param.get("grid", grid)) {
        if (!FESOM::match(grid, param).empty()) {
            return new FESOM(param);
        }

        if (!ICON::match(grid, param).empty()) {
            return new ICON(param);
        }

        if (!ORCA::match(grid, param).empty()) {
            return new ORCA(param);
        }
    }
#endif

    return new other::UnstructuredGrid(param);
}


namespace other {


UnstructuredGrid::UnstructuredGrid(const param::MIRParametrisation& parametrisation) {
    parametrisation.get("latitudes", latitudes_);
    parametrisation.get("longitudes", longitudes_);

    if (latitudes_.size() != longitudes_.size()) {
        throw exception::UserError("UnstructuredGrid: requires 'latitudes'/'longitudes' with the same size");
    }

    if (latitudes_.empty()) {
        size_t numberOfPoints = 0;
        parametrisation.get("numberOfPoints", numberOfPoints);

        if (numberOfPoints == 0) {
            throw exception::UserError("UnstructuredGrid: requires 'latitudes'/'longitudes' or 'numberOfPoints'");
        }

        // coordinates are unusable but unique
        latitudes_.assign(numberOfPoints, std::numeric_limits<double>::signaling_NaN());
        longitudes_.resize(numberOfPoints);
        std::iota(longitudes_.begin(), longitudes_.end(), 0);
    }

    util::check_duplicate_points("UnstructuredGrid from MIRParametrisation", latitudes_, longitudes_, parametrisation);
}


UnstructuredGrid::UnstructuredGrid(const eckit::PathName& path) {
    input::GriddefInput::load(path, latitudes_, longitudes_);
    util::check_duplicate_points("UnstructuredGrid from " + path.asString(), latitudes_, longitudes_);
}


void UnstructuredGrid::save(const eckit::PathName& path, const std::vector<double>& latitudes,
                            const std::vector<double>& longitudes, bool binary) {
    util::check_duplicate_points("UnstructuredGrid::save to " + path.asString(), latitudes, longitudes);
    ASSERT(binary);
    output::GriddefOutput::save(path, latitudes, longitudes);
}


UnstructuredGrid::UnstructuredGrid(const std::vector<double>& latitudes, const std::vector<double>& longitudes,
                                   const util::BoundingBox& bbox) :
    Gridded(bbox), latitudes_(latitudes), longitudes_(longitudes) {
    if (latitudes_.size() != longitudes_.size()) {
        throw exception::UserError("UnstructuredGrid: requires 'latitudes'/'longitudes' with the same size");
    }

    util::check_duplicate_points("UnstructuredGrid from arguments", latitudes_, longitudes_);
}


UnstructuredGrid::~UnstructuredGrid() = default;


void UnstructuredGrid::print(std::ostream& out) const {
    out << "UnstructuredGrid[points=" << numberOfPoints() << "]";
}


void UnstructuredGrid::makeName(std::ostream& out) const {
    out << "unstructured-" << numberOfPoints() << "-";

    eckit::MD5 hash;
    for (const auto& j : latitudes_) {
        hash << j;
    }
    for (const auto& j : longitudes_) {
        hash << j;
    }
    out << static_cast<std::string>(hash);
}


bool UnstructuredGrid::sameAs(const Representation& other) const {
    const auto* o = dynamic_cast<const UnstructuredGrid*>(&other);
    return (o != nullptr) && (latitudes_ == o->latitudes_) && (longitudes_ == o->longitudes_);
}


void UnstructuredGrid::fillGrib(grib_info& info) const {
    info.grid.grid_type        = CODES_UTIL_GRID_SPEC_UNSTRUCTURED;
    info.packing.editionNumber = 2;
}


void UnstructuredGrid::fillJob(api::MIRJob& job) const {
    job.set("latitudes", latitudes_);
    job.set("longitudes", longitudes_);
}


void UnstructuredGrid::fillMeshGen(util::MeshGeneratorParameters& params) const {
    if (params.meshGenerator_.empty()) {
        params.meshGenerator_ = "delaunay";
    }
}


util::Domain UnstructuredGrid::domain() const {
    // FIXME Should be global?
    return {bbox_.north(), bbox_.west(), bbox_.south(), bbox_.east()};
}


atlas::Grid UnstructuredGrid::atlasGrid() const {
    ASSERT(numberOfPoints());

    std::vector<atlas::PointXY> pts;
    pts.reserve(numberOfPoints());

    for (size_t i = 0; i < numberOfPoints(); ++i) {
        pts.emplace_back(longitudes_[i], latitudes_[i]);
    }

    return atlas::UnstructuredGrid(std::move(pts));
}


void UnstructuredGrid::validate(const MIRValuesVector& values) const {
    ASSERT_VALUES_SIZE_EQ_ITERATOR_COUNT("UnstructuredGrid", values.size(), numberOfPoints());
}


size_t UnstructuredGrid::numberOfPoints() const {
    return latitudes_.size();
}


const Gridded* UnstructuredGrid::croppedRepresentation(const util::BoundingBox& bbox) const {
    std::vector<double> lat;
    std::vector<double> lon;

    size_t i = 0;
    size_t j = 0;

    for (const std::unique_ptr<Iterator> iter(iterator()); iter->next(); ++i) {
        if (bbox.contains(iter->pointUnrotated())) {
            auto ip = iter->index();
            lat.emplace_back(latitudes_.at(ip));
            lon.emplace_back(longitudes_.at(ip));
            ++j;
        }
    }

    if (j < i) {
        Log::debug() << "UnstructuredGrid::croppedRepresentation: cropped " << Log::Pretty(i) << " to "
                     << Log::Pretty(j, {"point"}) << std::endl;
        ASSERT(j);
        return new UnstructuredGrid(lat, lon, bbox);
    }

    Log::debug() << "UnstructuredGrid::croppedRepresentation: no cropping" << std::endl;
    return this;
}


Iterator* UnstructuredGrid::iterator() const {
    return new iterator::UnstructuredIterator(latitudes_, longitudes_);
}


bool UnstructuredGrid::isPeriodicWestEast() const {
    return eckit::types::is_approximately_greater_or_equal(bbox_.east().value() - bbox_.west().value(),
                                                           Longitude::GLOBE.value());
}


bool UnstructuredGrid::includesNorthPole() const {
    return bbox_.north() == Latitude::NORTH_POLE;
}


bool UnstructuredGrid::includesSouthPole() const {
    return bbox_.south() == Latitude::SOUTH_POLE;
}


bool UnstructuredGrid::extendBoundingBoxOnIntersect() const {
    return false;
}


static const RepresentationBuilder<UnstructuredGrid> triangular_grid("triangular_grid");
static const RepresentationBuilder<UnstructuredGrid> unstructured_grid("unstructured_grid");


}  // namespace other
}  // namespace mir::repres
