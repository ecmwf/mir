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

#include <ostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "eckit/config/Resource.h"
#include "eckit/spec/Custom.h"
#include "eckit/utils/MD5.h"
#include "eckit/utils/StringTools.h"

#include "mir/api/MIRJob.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Iterator.h"
#include "mir/util/Domain.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Grib.h"
#include "mir/util/Log.h"
#include "mir/util/MeshGeneratorParameters.h"
#include "mir/util/UnsupportedFunctionality.h"


namespace mir::repres::regular {


RegularGrid::RegularGrid(const param::MIRParametrisation& param, const RegularGrid::Projection& projection) :
    shape_(param), xPlus_(true), yPlus_(false), firstPointBottomLeft_(false) {
    ASSERT(projection);

    auto get_long_first_key = [](const param::MIRParametrisation& param, const std::vector<std::string>& keys) -> long {
        long value = 0;
        for (const auto& key : keys) {
            if (param.get(key, value)) {
                return value;
            }
        }
        throw exception::SeriousBug("RegularGrid: couldn't find any key: " + eckit::StringTools::join(", ", keys));
    };

    long nx = get_long_first_key(param, {"numberOfPointsAlongXAxis", "Ni"});
    long ny = get_long_first_key(param, {"numberOfPointsAlongYAxis", "Nj"});
    ASSERT(nx > 0);
    ASSERT(ny > 0);

    std::vector<double> grid;
    ASSERT(param.get("grid", grid));
    ASSERT_KEYWORD_GRID_SIZE(grid.size());

    Point2 firstLL;
    ASSERT(param.get("latitudeOfFirstGridPointInDegrees", firstLL[LLCOORDS::LAT]));
    ASSERT(param.get("longitudeOfFirstGridPointInDegrees", firstLL[LLCOORDS::LON]));
    auto first = projection.xy(firstLL);

    param.get("iScansPositively", xPlus_);  // iScansPositively != 0
    param.get("jScansPositively", yPlus_);  // jScansPositively == 0
    param.get("first_point_bottom_left", firstPointBottomLeft_);

    x_    = linspace(first.x(), grid[0], nx, firstPointBottomLeft_ || xPlus_);
    y_    = linspace(first.y(), grid[1], ny, firstPointBottomLeft_ || yPlus_);
    grid_ = {x_, y_, projection};

    atlas::RectangularDomain range({x_.min(), x_.max()}, {y_.min(), y_.max()}, "meters");
    auto bbox = projection.lonlatBoundingBox(range);
    ASSERT(bbox);

    // MIR-661 Grid projection handling covering the poles: account for "excessive" bounds
    Longitude west(bbox.west());
    auto east = bbox.east() - bbox.west() >= Longitude::GLOBE.value() ? west + Longitude::GLOBE : bbox.east();

    bbox_ = {bbox.north(), west, bbox.south(), east};
}


RegularGrid::RegularGrid(const Projection& projection, const util::BoundingBox& bbox, const LinearSpacing& x,
                         const LinearSpacing& y, const util::Shape& shape) :
    Gridded(bbox),
    x_(x),
    y_(y),
    shape_(shape),
    xPlus_(x.front() <= x.back()),
    yPlus_(y.front() < y.back()),
    firstPointBottomLeft_(false) {
    grid_ = {x_, y_, projection};

    if (!shape_.provided) {
        shape_ = {grid_.projection().spec()};
    }
}


RegularGrid::~RegularGrid() = default;


RegularGrid::Projection::Spec RegularGrid::make_proj_spec(const param::MIRParametrisation& param) {
    static bool useProjIfAvailable = eckit::Resource<bool>("$MIR_USE_PROJ_IF_AVAILABLE", true);

    std::string proj;
    param.get("proj", proj);

    if (proj.empty() || !useProjIfAvailable || !::atlas::projection::ProjectionFactory::has("proj")) {
        return {};
    }

    Projection::Spec spec("type", "proj");
    spec.set("proj", proj);

    std::string projSource;
    if (param.get("projSource", projSource) && !projSource.empty()) {
        spec.set("proj_source", projSource);
    }

    std::string projGeocentric;
    if (param.get("projGeocentric", projGeocentric) && !projGeocentric.empty()) {
        spec.set("proj_geocentric", projGeocentric);
    }

    return spec;
}


RegularGrid::LinearSpacing RegularGrid::linspace(double start, double step, long num, bool plus) {
    ASSERT(step >= 0.);
    return {start, start + step * static_cast<double>(plus ? num - 1 : 1 - num), num};
}


void RegularGrid::print(std::ostream& out) const {
    out << "RegularGrid[x=" << x_.spec() << ",y=" << y_.spec() << ",projection=" << grid_.projection().spec()
        << ",firstPointBottomLeft=" << firstPointBottomLeft_ << ",bbox=" << bbox_ << "]";
}


size_t RegularGrid::numberOfPoints() const {
    return x_.size() * y_.size();
}


::atlas::Grid RegularGrid::atlasGrid() const {
    return grid_;
}


void RegularGrid::fillGrib(grib_info& info) const {
    // shape of the reference system
    shape_.fillGrib(info, grid_.projection().spec());

    // scanningMode
    info.grid.iScansNegatively = x_.front() < x_.back() ? 0L : 1L;
    info.grid.jScansPositively = y_.front() < y_.back() ? 1L : 0L;
}


void RegularGrid::fillSpec(CustomSpec& spec) const {
    // shape of the reference system
    shape_.fillSpec(spec, grid_.projection().spec());

    // scanningMode
    spec.set("iScansNegatively", static_cast<long>(x().front() < x().back() ? 0 : 1));
    spec.set("jScansPositively", static_cast<long>(y().front() < y().back() ? 1 : 0));
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


void RegularGrid::validate(const MIRValuesVector& values) const {
    const size_t count = numberOfPoints();

    Log::debug() << "RegularGrid::validate checked " << Log::Pretty(values.size(), {"value"}) << ", iterator counts "
                 << Log::Pretty(count) << " (" << domain() << ")." << std::endl;

    ASSERT_VALUES_SIZE_EQ_ITERATOR_COUNT("RegularGrid", values.size(), count);
}


Iterator* RegularGrid::iterator() const {
    class RegularGridIterator final : public Iterator {
        Projection projection_;
        const LinearSpacing& x_;
        const LinearSpacing& y_;
        PointLonLat pLonLat_;

        size_t ni_;
        size_t nj_;
        size_t i_     = 0;
        size_t j_     = 0;
        size_t count_ = 0;

        void print(std::ostream& out) const override {
            out << "RegularGridIterator[";
            Iterator::print(out);
            out << ",i=" << i_ << ",j=" << j_ << ",count=" << count_ << "]";
        }

        bool next(Latitude& _lat, Longitude& _lon) override {
            if (j_ < nj_ && i_ < ni_) {
                pLonLat_ = projection_.lonlat({x_[i_], y_[j_]});
                _lat     = lat(pLonLat_.lat());
                _lon     = lon(pLonLat_.lon());

                if (i_ > 0 || j_ > 0) {
                    count_++;
                }

                if (++i_ == ni_) {
                    i_ = 0;
                    j_++;
                }

                return true;
            }
            return false;
        }

        size_t index() const override { return count_; }

    public:
        RegularGridIterator(Projection projection, const LinearSpacing& x, const LinearSpacing& y) :
            projection_(std::move(projection)), x_(x), y_(y), ni_(x.size()), nj_(y.size()) {}
    };

    return new RegularGridIterator(grid_.projection(), x_, y_);
}


void RegularGrid::makeName(std::ostream& out) const {
    eckit::MD5 h;
    h << grid_.projection().spec();
    h << x_.spec();
    h << y_.spec();
    h << firstPointBottomLeft_;
    if (shape_.provided) {
        h << shape_.code;
        h << shape_.a;
        h << shape_.b;
    }
    auto type = grid_.projection().spec().getString("type");
    out << "RegularGrid-" << (type.empty() ? "" : type + "-") << h.digest();
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


// -----------------------------------------------------------------------------


struct UnsupportedRegularGrid : RegularGrid, util::UnsupportedFunctionality {
    static const RegularGrid::Projection PROJECTION_NONE;
    explicit UnsupportedRegularGrid(const param::MIRParametrisation& p) :
        RegularGrid(p, PROJECTION_NONE), util::UnsupportedFunctionality("grid " + [this]() {
            std::ostringstream ss;
            this->print(ss);
            return ss.str();
        }() + " is currently unsupported") {}
};


const RegularGrid::Projection UnsupportedRegularGrid::PROJECTION_NONE;


struct Albers final : UnsupportedRegularGrid {
    using UnsupportedRegularGrid::UnsupportedRegularGrid;
    void print(std::ostream& out) const override { out << "Albers[]"; }
};


struct AzimuthRange final : UnsupportedRegularGrid {
    using UnsupportedRegularGrid::UnsupportedRegularGrid;
    void print(std::ostream& out) const override { out << "AzimuthRange[]"; }
};


struct EquatorialAzimuthalEquidistant final : UnsupportedRegularGrid {
    using UnsupportedRegularGrid::UnsupportedRegularGrid;
    void print(std::ostream& out) const override { out << "EquatorialAzimuthalEquidistant[]"; }
};


struct StretchedGG final : UnsupportedRegularGrid {
    using UnsupportedRegularGrid::UnsupportedRegularGrid;
    void print(std::ostream& out) const override { out << "StretchedGG[]"; }
};


struct StretchedLL final : UnsupportedRegularGrid {
    using UnsupportedRegularGrid::UnsupportedRegularGrid;
    void print(std::ostream& out) const override { out << "StretchedLL[]"; }
};


struct StretchedRotatedGG final : UnsupportedRegularGrid {
    using UnsupportedRegularGrid::UnsupportedRegularGrid;
    void print(std::ostream& out) const override { out << "StretchedRotatedGG[]"; }
};


struct StretchedRotatedLL final : UnsupportedRegularGrid {
    using UnsupportedRegularGrid::UnsupportedRegularGrid;
    void print(std::ostream& out) const override { out << "StretchedRotatedLL[]"; }
};


struct TransverseMercator final : UnsupportedRegularGrid {
    using UnsupportedRegularGrid::UnsupportedRegularGrid;
    void print(std::ostream& out) const override { out << "TransverseMercator[]"; }
};


static const RepresentationBuilder<Albers> ALBERS("albers");
static const RepresentationBuilder<AzimuthRange> AZIMUTH_RANGE("azimuth_range");
static const RepresentationBuilder<EquatorialAzimuthalEquidistant> EQUATORIAL_AZ_EQ("equatorial_azimuthal_equidistant");
static const RepresentationBuilder<StretchedGG> STRETCHED_GG("stretched_gg");
static const RepresentationBuilder<StretchedLL> STRETCHED_LL("stretched_ll");
static const RepresentationBuilder<StretchedRotatedGG> STRETCHED_ROTATED_GG("stretched_rotated_gg");
static const RepresentationBuilder<StretchedRotatedLL> STRETCHED_ROTATED_LL("stretched_rotated_ll");
static const RepresentationBuilder<TransverseMercator> TRANSVERSE_MERCATOR("transverse_mercator");


}  // namespace mir::repres::regular
