// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/repres/RegularXY.h"

#include <cmath>
#include <ostream>
#include <string>
#include <utility>
#include <variant>

#include "eckit/geo/Figure.h"
#include "eckit/geo/Grid.h"
#include "eckit/geo/Projection.h"
#include "eckit/geo/Range.h"
#include "eckit/geo/figure/Earth.h"
#include "eckit/parser/YAMLParser.h"
#include "eckit/spec/Custom.h"
#include "eckit/utils/MD5.h"

#include "mir/iterator/GridIterator.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Iterator.h"
#include "mir/util/Angles.h"
#include "mir/util/Atlas.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Grib.h"
#include "mir/util/MeshGeneratorParameters.h"


namespace mir::repres {


namespace {


using eckit::geo::Figure;
using eckit::geo::PointLonLat;
using eckit::geo::PointXY;
using CustomSpec = eckit::spec::Custom;


double get_double(const param::MIRParametrisation& param, const std::string& key) {
    double value = 0.;
    if (!param.get(key, value)) {
        throw exception::UserError("RegularXY: '" + key + "' required");
    }
    return value;
}


double get_double(const param::MIRParametrisation& param, const std::string& key, double dfault) {
    param.get(key, dfault);
    return dfault;
}


long get_edition(const param::MIRParametrisation& param) {
    // GRIB2 unless specified, as encoding any figure
    long edition = 2;
    param.get("edition", edition);
    return edition;
}


long set_edition(grib_info& info, long dfault) {
    // the edition specified (as option, or by the representation), otherwise set
    static const std::string EDITION{"edition"};
    for (long j = 0; j < info.packing.extra_settings_count; ++j) {
        if (const auto& set = info.packing.extra_settings[j]; set.name == EDITION && set.type == CODES_TYPE_LONG) {
            return set.long_value;
        }
    }

    if (info.packing.editionNumber == 0) {
        info.packing.editionNumber = dfault;
    }

    return info.packing.editionNumber;
}


Figure* make_figure(const param::MIRParametrisation& param) {
    // as encoded [m] (GRIB), or as given (typed grids), if given
    auto get = [&param](const std::string& key, double& value) {
        return param.get(key + "InMetres", value) || param.get(key, value);
    };

    if (double a = 0., b = 0.; get("earthMajorAxis", a) && get("earthMinorAxis", b)) {
        return eckit::geo::FigureFactory::build(CustomSpec{{"a", a}, {"b", b}});
    }

    if (double R = 0.; get("radius", R)) {
        return eckit::geo::FigureFactory::build(CustomSpec{{"R", R}});
    }

    return nullptr;
}


long shape_of_the_earth(const Figure& figure) {
    // GRIB2 code table 3.2, by the figure (as ecCodes)
    using namespace eckit::geo::figure;
    struct code_t {
        long code;
        std::shared_ptr<const Figure> figure;
    };

    static const std::vector<code_t> CODES{
        {0, std::make_shared<EarthGrib1>()}, {2, std::make_shared<EarthIau1965>()},
        {4, std::make_shared<EarthGrs80>()}, {5, std::make_shared<EarthWgs84>()},
        {6, std::make_shared<Earth>()},      {8, std::make_shared<EarthWgs84Sphere>()},
    };

    for (const auto& c : CODES) {
        if (figure == *c.figure) {
            return c.code;
        }
    }

    return figure.spherical() ? 1 : 7;
}


CustomSpec* make_spec(const param::MIRParametrisation& param, std::unique_ptr<CustomSpec> projection) {
    std::unique_ptr<const Figure> figure(make_figure(param));

    auto set_figure = [&figure](CustomSpec& projection) {
        if (figure && projection.get_string("type", "") != "proj") {
            const std::vector<std::string> keys{"figure", "R", "r", "a", "b"};
            for (const auto& key : keys) {
                projection.erase(key);
            }
            figure->fill_spec(projection);
        }
    };

    // by the gridSpec (eg. GRIB), with the figure as encoded (if)
    if (std::string gridspec; param.get("gridSpec", gridspec)) {
        std::unique_ptr<CustomSpec> spec(CustomSpec::make_from_value(eckit::YAMLParser::decodeString(gridspec)));
        if (spec->has_custom("projection")) {
            set_figure(*spec->custom("projection"));
        }
        return spec.release();
    }

    // by the keys of a typed grid: the projection (PROJ, or by type) and the first point
    auto proj = std::make_unique<CustomSpec>();
    if (std::string str; param.get("proj", str) && !str.empty()) {
        proj->set("type", "proj");
        proj->set("target", str);
    }
    else if (projection) {
        if (!figure) {
            figure = std::make_unique<eckit::geo::figure::Earth>();
        }
        proj = std::move(projection);
        set_figure(*proj);
    }
    else {
        std::string type;
        param.get("gridType", type);
        throw exception::FunctionalityNotSupported("RegularXY: gridType=" + type + " by gridSpec only");
    }

    long nx = 0;
    long ny = 0;
    ASSERT(param.get("numberOfPointsAlongXAxis", nx) || param.get("Ni", nx));
    ASSERT(param.get("numberOfPointsAlongYAxis", ny) || param.get("Nj", ny));
    ASSERT(nx > 0 && ny > 0);

    std::vector<double> grid;
    ASSERT(param.get("grid", grid));
    ASSERT_KEYWORD_GRID_SIZE(grid.size());
    ASSERT(grid[0] > 0. && grid[1] > 0.);

    const std::unique_ptr<const eckit::geo::Projection> p(eckit::geo::ProjectionFactory::build(*proj));
    const auto first = std::get<PointXY>(p->fwd(PointLonLat{get_double(param, "longitudeOfFirstGridPointInDegrees"),
                                                            get_double(param, "latitudeOfFirstGridPointInDegrees")}));

    bool bottomLeft = false;
    bool xPlus      = true;   // iScansPositively != 0
    bool yPlus      = false;  // jScansPositively == 0
    param.get("first_point_bottom_left", bottomLeft);
    param.get("iScansPositively", xPlus);
    param.get("jScansPositively", yPlus);

    auto range = [](double first, double inc, long n, bool plus) {
        const auto d = plus ? inc : -inc;
        return std::vector<double>{first, first + d * static_cast<double>(n - 1), d};
    };

    proj->set("reverse", true);

    auto* spec = new CustomSpec;
    spec->set("x", range(first.X(), grid[0], nx, bottomLeft || xPlus));
    spec->set("y", range(first.Y(), grid[1], ny, bottomLeft || yPlus));
    spec->set("projection", proj.release());
    return spec;
}


util::BoundingBox bounding_box(const eckit::geo::Grid& grid) {
    const auto bbox = grid.boundingBox();
    const auto e    = bbox.east();
    const auto w    = bbox.west();

    // MIR-661 Grid projection handling covering the poles: account for "excessive" bounds
    Longitude west(w);
    return {bbox.north(), west, bbox.south(),
            e - w >= Longitude::GLOBE.value() ? west + Longitude::GLOBE : Longitude(e)};
}


}  // namespace


RegularXY::RegularXY(const param::MIRParametrisation& param, std::unique_ptr<CustomSpec> projection) :
    edition_(get_edition(param)) {
    param.get("first_point_bottom_left", firstPointBottomLeft_);

    std::unique_ptr<const CustomSpec> spec(make_spec(param, std::move(projection)));
    grid_.reset(eckit::geo::GridFactory::build(*spec));
    ASSERT(grid_);

    // false easting/northing, as the grid
    const auto f = grid_->false_xy();
    false_       = {f.X(), f.Y()};

    bbox_ = bounding_box(*grid_);
}


RegularXY::~RegularXY() = default;


const eckit::geo::Grid& RegularXY::grid() const {
    return *grid_;
}


Point2 RegularXY::lonlat(double x, double y) const {
    const auto p = std::get<PointLonLat>(grid_->projection().fwd(PointXY{x - false_[0], y - false_[1]}));
    return {p.lon(), p.lat()};
}


Point2 RegularXY::firstPoint() const {
    if (firstPointBottomLeft_) {
        const auto& x = grid_->x();
        const auto& y = grid_->y();
        return lonlat(std::min(x.a(), x.b()), std::min(y.a(), y.b()));
    }

    const auto p = std::get<PointLonLat>(grid_->first_point());
    return {p.lon(), p.lat()};
}


long RegularXY::gribEdition(grib_info& info) const {
    return set_edition(info, edition_);
}


void RegularXY::print(std::ostream& out) const {
    out << "RegularXY[" << spec().str() << "]";
}


size_t RegularXY::numberOfPoints() const {
    return grid_->size();
}


::atlas::Grid RegularXY::atlasGrid() const {
    // atlas projection by PROJ (as the eckit::geo projection), otherwise by the points
    if (::atlas::projection::ProjectionFactory::has("proj")) {
        const auto& gx = grid_->x();
        const auto& gy = grid_->y();
        ::atlas::grid::LinearSpacing x(gx.a() - false_[0], gx.b() - false_[0], static_cast<long>(gx.size()));
        ::atlas::grid::LinearSpacing y(gy.a() - false_[1], gy.b() - false_[1], static_cast<long>(gy.size()));
        ::atlas::Projection projection(
            ::atlas::Projection::Spec("type", "proj").set("proj", grid_->projection().proj_str()));
        return ::atlas::RegularGrid(x, y, projection);
    }

    const auto latlons = grid_->to_latlons();
    const auto& lats   = latlons.first;
    const auto& lons   = latlons.second;

    std::vector<::atlas::PointXY> points(lats.size());
    for (size_t i = 0; i < points.size(); ++i) {
        points[i].assign(lons[i], lats[i]);
    }
    return ::atlas::UnstructuredGrid(std::move(points));
}


void RegularXY::fillGrib(grib_info& info) const {
    // the figure (GRIB2 code table 3.2, GRIB1 resolution and component flags, without a custom figure provision)
    const auto& figure = grid_->projection().figure();
    if (gribEdition(info) == 1) {
        const auto oblate = figure == eckit::geo::figure::EarthIau1965{};
        if (!oblate && figure != eckit::geo::figure::EarthGrib1{}) {
            throw exception::FunctionalityNotSupported("RegularXY: GRIB1 encoding of the figure " + figure.spec_str());
        }
        info.extra_set("earthIsOblate", oblate ? 1L : 0L);
    }
    else {
        const auto code = shape_of_the_earth(figure);
        info.extra_set("shapeOfTheEarth", code);

        if (code == 1) {
            info.extra_set("radius", figure.R());
        }
        else if (code == 7) {
            info.extra_set("earthMajorAxis", figure.a());
            info.extra_set("earthMinorAxis", figure.b());
        }
    }

    // scanningMode
    info.grid.iScansNegatively = grid_->x().a() < grid_->x().b() ? 0L : 1L;
    info.grid.jScansPositively = grid_->y().a() < grid_->y().b() ? 1L : 0L;
}


bool RegularXY::includesNorthPole() const {
    return bbox_.north() == Latitude::NORTH_POLE;
}


bool RegularXY::includesSouthPole() const {
    return bbox_.south() == Latitude::SOUTH_POLE;
}


bool RegularXY::isPeriodicWestEast() const {
    return includesNorthPole() || includesSouthPole() ||
           (bbox_.east().value() - bbox_.west().value() >= Longitude::GLOBE.value());
}


void RegularXY::validate(const MIRValuesVector& values) const {
    ASSERT_VALUES_SIZE_EQ_ITERATOR_COUNT("RegularXY", values.size(), numberOfPoints());
}


Iterator* RegularXY::iterator() const {
    return new iterator::GridIterator(*grid_);
}


void RegularXY::makeName(std::ostream& out) const {
    // the spec, and the figure (as default figures are not in the spec)
    eckit::MD5 h;
    h << spec().str();
    h << grid_->projection().figure().spec_str();
    out << "RegularXY-" << h.digest();
}


bool RegularXY::sameAs(const Representation& other) const {
    const auto* o = dynamic_cast<const RegularXY*>(&other);
    return (o != nullptr) && uniqueName() == o->uniqueName();
}


void RegularXY::fillMeshGen(util::MeshGeneratorParameters& params) const {
    if (params.meshGenerator_.empty()) {
        params.meshGenerator_ = ::atlas::projection::ProjectionFactory::has("proj") ? "structured" : "delaunay";
    }
}


// -----------------------------------------------------------------------------


namespace {


/// Settings of the GRIB encoding, as the edition allows: GRIB1 cannot write LaD, GRIB2 cannot write negative longitudes
struct Encoding {
    explicit Encoding(const param::MIRParametrisation& param) {
        param.get("writeLaDInDegrees", writeLaDInDegrees = true);
        param.get("writeLonPositive", writeLonPositive = false);

        long uv          = 0;
        uvRelativeToGrid = param.get("uvRelativeToGrid", uv) && uv != 0;
    }

    double longitude(double lon, long edition) const {
        return writeLonPositive || edition == 2 ? util::normalise_longitude(lon, 0) : lon;
    }

    bool LaD(long edition) const { return writeLaDInDegrees && edition == 2; }

    bool writeLaDInDegrees = true;
    bool writeLonPositive  = false;
    bool uvRelativeToGrid  = false;
};


class Lambert final : public RegularXY {
public:
    explicit Lambert(const param::MIRParametrisation& param) :
        RegularXY(param, projection(param)),
        encoding_(param),
        LaD_(get_double(param, "LaDInDegrees")),
        LoV_(get_double(param, "LoVInDegrees")),
        Latin1_(get_double(param, "Latin1InDegrees", LaD_)),
        Latin2_(get_double(param, "Latin2InDegrees", LaD_)),
        southPoleLat_(get_double(param, "latitudeOfSouthernPoleInDegrees", -90.)),
        southPoleLon_(get_double(param, "longitudeOfSouthernPoleInDegrees", 0.)) {}

private:
    const Encoding encoding_;
    const double LaD_;
    const double LoV_;
    const double Latin1_;
    const double Latin2_;
    const double southPoleLat_;
    const double southPoleLon_;

    static std::unique_ptr<CustomSpec> projection(const param::MIRParametrisation& param) {
        const auto LaD = get_double(param, "LaDInDegrees");
        return std::unique_ptr<CustomSpec>(new CustomSpec{{"type", "lcc"},
                                                          {"lat_0", LaD},
                                                          {"lat_1", get_double(param, "Latin1InDegrees", LaD)},
                                                          {"lat_2", get_double(param, "Latin2InDegrees", LaD)},
                                                          {"lon_0", get_double(param, "LoVInDegrees")}});
    }

    void fillGrib(grib_info& info) const override {
        info.grid.grid_type = CODES_UTIL_GRID_SPEC_LAMBERT_CONFORMAL;

        const auto edition = gribEdition(info);
        const auto first   = firstPoint();

        info.grid.latitudeOfFirstGridPointInDegrees  = first[LLCOORDS::LAT];
        info.grid.longitudeOfFirstGridPointInDegrees = encoding_.longitude(first[LLCOORDS::LON], edition);

        info.grid.Ni = static_cast<long>(grid().nx());
        info.grid.Nj = static_cast<long>(grid().ny());

        info.grid.latitudeOfSouthernPoleInDegrees  = southPoleLat_;
        info.grid.longitudeOfSouthernPoleInDegrees = southPoleLon_;
        info.grid.uvRelativeToGrid                 = encoding_.uvRelativeToGrid ? 1 : 0;

        info.extra_set("DxInMetres", std::abs(grid().dx()));
        info.extra_set("DyInMetres", std::abs(grid().dy()));
        info.extra_set("Latin1InDegrees", Latin1_);
        info.extra_set("Latin2InDegrees", Latin2_);
        info.extra_set("LoVInDegrees", encoding_.longitude(LoV_, edition));

        if (encoding_.LaD(edition)) {
            info.extra_set("LaDInDegrees", LaD_);
        }

        RegularXY::fillGrib(info);
    }
};


class LambertAzimuthalEqualArea final : public RegularXY {
public:
    explicit LambertAzimuthalEqualArea(const param::MIRParametrisation& param) :
        RegularXY(param, projection(param)),
        standardParallel_(get_double(param, "standardParallelInDegrees")),
        centralLongitude_(get_double(param, "centralLongitudeInDegrees")) {}

private:
    const double standardParallel_;
    const double centralLongitude_;

    static std::unique_ptr<CustomSpec> projection(const param::MIRParametrisation& param) {
        return std::unique_ptr<CustomSpec>(new CustomSpec{{"type", "laea"},
                                                          {"lat_0", get_double(param, "standardParallelInDegrees")},
                                                          {"lon_0", get_double(param, "centralLongitudeInDegrees")}});
    }

    void fillGrib(grib_info& info) const override {
        info.grid.grid_type        = CODES_UTIL_GRID_SPEC_LAMBERT_AZIMUTHAL_EQUAL_AREA;
        info.packing.editionNumber = 2;

        const auto first = firstPoint();

        info.grid.Ni = static_cast<long>(grid().nx());
        info.grid.Nj = static_cast<long>(grid().ny());

        info.grid.latitudeOfFirstGridPointInDegrees  = first[LLCOORDS::LAT];
        info.grid.longitudeOfFirstGridPointInDegrees = first[LLCOORDS::LON];

        info.extra_set("DxInMetres", std::abs(grid().dx()));
        info.extra_set("DyInMetres", std::abs(grid().dy()));
        info.extra_set("standardParallelInDegrees", standardParallel_);
        info.extra_set("centralLongitudeInDegrees", centralLongitude_);

        RegularXY::fillGrib(info);
    }
};


class PolarStereographic final : public RegularXY {
public:
    explicit PolarStereographic(const param::MIRParametrisation& param) :
        RegularXY(param, projection(param)),
        encoding_(param),
        LaD_(get_double(param, "LaDInDegrees", 60.)),
        orientation_(get_double(param, "orientationOfTheGridInDegrees", 0.)) {}

private:
    const Encoding encoding_;
    const double LaD_;
    const double orientation_;

    static bool south_pole(const param::MIRParametrisation& param) {
        long south = 0;
        return param.get("southPoleOnProjectionPlane", south) && south != 0;
    }

    static std::unique_ptr<CustomSpec> projection(const param::MIRParametrisation& param) {
        return std::unique_ptr<CustomSpec>(
            new CustomSpec{{"type", "stere"},
                           {"lat_0", south_pole(param) ? -90. : 90.},
                           {"lat_ts", get_double(param, "LaDInDegrees", 60.)},
                           {"lon_0", get_double(param, "orientationOfTheGridInDegrees", 0.)}});
    }

    void fillGrib(grib_info& info) const override {
        info.grid.grid_type = CODES_UTIL_GRID_SPEC_POLAR_STEREOGRAPHIC;

        const auto edition = gribEdition(info);
        const auto first   = firstPoint();

        info.grid.latitudeOfFirstGridPointInDegrees  = first[LLCOORDS::LAT];
        info.grid.longitudeOfFirstGridPointInDegrees = encoding_.longitude(first[LLCOORDS::LON], edition);

        info.grid.Ni = static_cast<long>(grid().nx());
        info.grid.Nj = static_cast<long>(grid().ny());

        info.grid.uvRelativeToGrid = encoding_.uvRelativeToGrid ? 1 : 0;

        info.extra_set("DxInMetres", std::abs(grid().dx()));
        info.extra_set("DyInMetres", std::abs(grid().dy()));
        info.extra_set("orientationOfTheGridInDegrees", util::normalise_longitude(orientation_, 0));

        if (encoding_.LaD(edition)) {
            info.extra_set("LaDInDegrees", LaD_);
        }

        RegularXY::fillGrib(info);
    }
};


/// Grids by gridSpec only (eg. GRIB), without GRIB encoding
class RegularXYDecodedOnly : public RegularXY {
public:
    using RegularXY::RegularXY;

private:
    void fillGrib(grib_info& /*unused*/) const override {
        throw exception::FunctionalityNotSupported("RegularXY: GRIB encoding of " + spec().str());
    }
};


class SpaceView final : public RegularXYDecodedOnly {
public:
    explicit SpaceView(const param::MIRParametrisation& param) : RegularXYDecodedOnly(param) {
        // longest element diagonal, a multiple of a reference central element diagonal (avoiding distortion), about
        // the sub-satellite point (the projection origin)
        const auto dx = grid().dx();
        const auto dy = grid().dy();
        const auto a  = lonlat(-dx / 2., dy / 2.);
        const auto b  = lonlat(dx / 2., -dy / 2.);

        longestElementDiagonal_ =
            20. * util::Earth::distance(::atlas::PointLonLat(a[0], a[1]), ::atlas::PointLonLat(b[0], b[1]));
        ASSERT(0. < longestElementDiagonal_);
    }

private:
    double longestElementDiagonal_ = 0.;

    bool getLongestElementDiagonal(double& d) const override {
        d = longestElementDiagonal_;
        return true;
    }
};


const RepresentationBuilder<Lambert> LAMBERT("lambert");
const RepresentationBuilder<LambertAzimuthalEqualArea> LAMBERT_AZIMUTHAL_EQUAL_AREA("lambert_azimuthal_equal_area");
const RepresentationBuilder<PolarStereographic> POLAR_STEREOGRAPHIC("polar_stereographic");
const RepresentationBuilder<RegularXYDecodedOnly> ALBERS("albers");
const RepresentationBuilder<RegularXYDecodedOnly> MERCATOR("mercator");
const RepresentationBuilder<RegularXYDecodedOnly> TRANSVERSE_MERCATOR("transverse_mercator");
const RepresentationBuilder<SpaceView> SPACE_VIEW("space_view");
const RepresentationBuilder<RegularXYDecodedOnly> REGULAR_XY("regular_xy");


}  // namespace


}  // namespace mir::repres
