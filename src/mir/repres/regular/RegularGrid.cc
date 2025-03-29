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
#include "eckit/geo/spec/Custom.h"
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


namespace mir::repres::regular {


const RegularGrid::LinearSpacing::Spec& RegularGrid::LinearSpacing::spec() const {
    if (!spec_) {
        spec_ = std::make_unique<eckit::geo::spec::Custom>();
        spec_->set("type", "linear");
        spec_->set("endpoint", endpoint_);
        spec_->set("start", front());
        spec_->set("end", back());
        spec_->set("size", size());
    }
    return *spec_;
}


RegularGrid::LinearSpacing::LinearSpacing(value_type a, value_type b, long n, bool endpoint) :
    vector(static_cast<size_t>(n)), endpoint_(endpoint) {
    ASSERT(n > 1);
    eckit::Fraction dx((b - a) / static_cast<double>(n - (endpoint ? 1 : 0)));
    eckit::Fraction x(a);

    resize(static_cast<size_t>(n));
    for (auto& v : *this) {
        v = x;
        x += dx;
    }
}


RegularGrid::RegularGrid(const param::MIRParametrisation& param, eckit::geo::Spec* projection) :
    shape_(param),
    projection_(Projection::make_from_spec(*std::unique_ptr<eckit::geo::Spec>(projection))),
    xPlus_(true),
    yPlus_(false),
    firstPointBottomLeft_(false) {

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

    double firstLat = 0;
    double firstLon = 0;
    ASSERT(param.get("latitudeOfFirstGridPointInDegrees", firstLat));
    ASSERT(param.get("longitudeOfFirstGridPointInDegrees", firstLon));
    auto first = std::get<PointXY>(projection_->fwd(PointLonLat{firstLon, firstLat}));

    param.get("iScansPositively", xPlus_);  // iScansPositively != 0
    param.get("jScansPositively", yPlus_);  // jScansPositively == 0
    param.get("first_point_bottom_left", firstPointBottomLeft_);

    x_ = linspace(first.X, grid[0], nx, firstPointBottomLeft_ || xPlus_);
    y_ = linspace(first.Y, grid[1], ny, firstPointBottomLeft_ || yPlus_);

    // use [0, 360[ longitude range if periodic
    // MIR-661 Grid projection handling covering the poles: account for "excessive" bounds
    std::unique_ptr<eckit::geo::area::BoundingBox> bbox(eckit::geo::area::BoundingBox::make_from_projection(
        PointXY{x_.min(), y_.min()}, PointXY{x_.max(), y_.max()}, *projection_));
    bbox_ = {
        bbox->north,
        bbox->periodic() ? Longitude::GREENWICH : bbox->west,
        bbox->south,
        bbox->periodic()                                      ? Longitude::GLOBE.value()
        : bbox->east - bbox->west >= Longitude::GLOBE.value() ? bbox->west + Longitude::GLOBE.value()
                                                              : bbox->east,
    };
}


RegularGrid::RegularGrid(Projection* projection, const util::BoundingBox& bbox, const LinearSpacing& x,
                         const LinearSpacing& y, const util::Shape& shape) :
    Gridded(bbox),
    projection_(projection),
    x_(x),
    y_(y),
    shape_(shape),
    xPlus_(x.front() <= x.back()),
    yPlus_(y.front() < y.back()),
    firstPointBottomLeft_(false) {
    ASSERT(projection_);

    if (!shape_.provided) {
        shape_ = util::Shape{*projection};
    }
}


RegularGrid::~RegularGrid() = default;


eckit::geo::spec::Custom* RegularGrid::make_proj_spec(const param::MIRParametrisation& param) {
    static bool useProjIfAvailable = eckit::Resource<bool>("$MIR_USE_PROJ_IF_AVAILABLE", true);

    std::string proj;
    param.get("proj", proj);

    if (proj.empty() || !useProjIfAvailable) {
        return {};
    }

    auto* spec = new eckit::geo::spec::Custom{{"type", "proj"}, {"proj", proj}};

    if (std::string projSource; param.get("projSource", projSource) && !projSource.empty()) {
        spec->set("proj_source", projSource);
    }

    if (std::string projGeocentric; param.get("projGeocentric", projGeocentric) && !projGeocentric.empty()) {
        spec->set("proj_geocentric", projGeocentric);
    }

    return spec;
}


RegularGrid::LinearSpacing RegularGrid::linspace(double start, double step, long num, bool plus) {
    ASSERT(step >= 0.);
    return LinearSpacing(start, start + step * static_cast<double>(plus ? num - 1 : 1 - num), num);
}


void RegularGrid::print(std::ostream& out) const {
    out << "RegularGrid[x=" << x_.spec() << ",y=" << y_.spec() << ",projection=" << projection_->spec()
        << ",firstPointBottomLeft=" << firstPointBottomLeft_ << ",bbox=" << bbox_ << "]";
}


bool RegularGrid::extendBoundingBoxOnIntersect() const {
    return true;
}


size_t RegularGrid::numberOfPoints() const {
    return x_.size() * y_.size();
}


::atlas::Grid RegularGrid::atlasGrid() const {
    NOTIMP;
}


void RegularGrid::fillGrib(grib_info& info) const {
    // shape of the reference system
    shape_.fillGrib(info, projection());

    // scanningMode
    info.grid.iScansNegatively = x_.front() < x_.back() ? 0L : 1L;
    info.grid.jScansPositively = y_.front() < y_.back() ? 1L : 0L;
}


void RegularGrid::fillJob(api::MIRJob& job) const {
    // shape of the reference system
    shape_.fillJob(job, projection());

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
    class RegularGridIterator : public Iterator {
        const Projection& projection_;
        const LinearSpacing& x_;
        const LinearSpacing& y_;

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
                auto p = std::get<PointLonLat>(projection_.inv(PointXY{x_[i_], y_[j_]}));
                _lat   = lat(p.lat);
                _lon   = lon(p.lon);

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
        RegularGridIterator(const Projection& projection, const LinearSpacing& x, const LinearSpacing& y) :
            projection_(std::move(projection)), x_(x), y_(y), ni_(x.size()), nj_(y.size()) {}
        ~RegularGridIterator() override = default;

        RegularGridIterator(const RegularGridIterator&)            = delete;
        RegularGridIterator(RegularGridIterator&&)                 = delete;
        RegularGridIterator& operator=(const RegularGridIterator&) = delete;
        RegularGridIterator& operator=(RegularGridIterator&&)      = delete;
    };

    return new RegularGridIterator(projection(), x_, y_);
}


const RegularGrid::Projection& RegularGrid::projection() const {
    ASSERT(projection_);
    return *projection_;
}


void RegularGrid::makeName(std::ostream& out) const {
    eckit::MD5 h;
    h << projection_->spec().str();
    h << x_.spec().str();
    h << y_.spec().str();
    h << firstPointBottomLeft_;
    if (shape_.provided) {
        h << shape_.code;
        h << shape_.a;
        h << shape_.b;
    }
    auto type = projection_->spec().get_string("type");
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


}  // namespace mir::repres::regular
