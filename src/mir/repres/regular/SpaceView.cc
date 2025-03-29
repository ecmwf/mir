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


#include "mir/repres/regular/SpaceView.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <limits>
#include <ostream>
#include <string>
#include <vector>

#include "eckit/geo/projection/PROJ.h"
#include "eckit/geo/spec/Custom.h"
#include "eckit/types/FloatCompare.h"

#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Iterator.h"
#include "mir/util/Angles.h"
#include "mir/util/Earth.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Trace.h"


namespace mir::repres::regular {


static const RepresentationBuilder<SpaceView> __builder("space_view");


template <typename EXTERNAL_T, typename INTERNAL_T = EXTERNAL_T>
EXTERNAL_T get(const param::MIRParametrisation& param, const std::string& key) {
    INTERNAL_T value;
    ASSERT(param.get(key, value));
    return static_cast<EXTERNAL_T>(value);
}


SpaceView::SpaceViewInternal::SpaceViewInternal(const param::MIRParametrisation& param) {
    auto earthIsOblate = get<bool>(param, "earthIsOblate");
    auto a             = get<double>(param, earthIsOblate ? "earthMajorAxis" : "radius");
    auto b             = earthIsOblate ? get<double>(param, "earthMinorAxis") : a;

    auto Nr = get<double>(param, "NrInRadiusOfEarthScaled");
    ASSERT(Nr > 1.);

    auto h = (Nr - 1.) * a;

    Lop      = get<double>(param, "longitudeOfSubSatellitePointInDegrees");
    auto Lap = get<double>(param, "latitudeOfSubSatellitePointInDegrees");
    ASSERT(eckit::types::is_approximately_equal(Lap, 0.));

    // ASSERT(get<size_t>(param, "orientationOfTheGridInDegrees") == 180);


    // projection
    auto proj = [](double h, double a, double b, double lon_0) {
        auto _h = " +h=" + std::to_string(h);
        auto _l = eckit::types::is_approximately_equal(lon_0, 0.) ? "" : " +lon_0=" + std::to_string(lon_0);
        auto _e = eckit::types::is_approximately_equal(a, b) ? " +R=" + std::to_string(a)
                                                             : " +a=" + std::to_string(a) + " +b=" + std::to_string(b);
        return "+proj=geos +type=crs +sweep=y" + _h + _l + _e;
    };

    using PROJ = eckit::geo::projection::PROJ;
    using Spec = eckit::geo::spec::Custom;

    projection.reset(new PROJ(Spec{{"type", "proj"}, {"proj", proj(h, a, b, Lop)}}));
    projectionGreenwich.reset(new PROJ(Spec{{"type", "proj"}, {"proj", proj(h, a, b, 0)}}));
    shape = util::Shape(*projection);

    // (x, y) space
    auto Nx_ = get<long>(param, "Nx");
    ASSERT(1 < Nx_);

    auto ip = get<bool>(param, "iScansPositively");
    auto xp = get<double, long>(param, "XpInGridLengths");
    auto dx = get<double, long>(param, "dx");
    ASSERT(dx > 0);

    auto rx = 2. * std::asin(1. / Nr) / dx * h;  // (height factor is PROJ-specific)

    double xa_ = ip ? rx * (-xp) : rx * (-xp + double(Nx_ - 1));
    double xb_ = ip ? rx * (-xp + double(Nx_ - 1)) : rx * (-xp);

    auto Ny_ = get<long>(param, "Ny");
    ASSERT(1 < Ny_);

    auto jp = get<bool>(param, "jScansPositively");
    auto yp = get<double, long>(param, "YpInGridLengths");
    auto dy = get<double, long>(param, "dy");
    ASSERT(dy > 0);

    auto ry = 2. * std::asin(1. / Nr) / dy * h;  // (height factor is PROJ-specific)

    double ya_ = jp ? ry * (-yp) : ry * (-yp + double(Ny_ - 1));
    double yb_ = jp ? ry * (-yp + double(Ny_ - 1)) : ry * (-yp);


    // longest element diagonal, a multiple of a reference central element diagonal (avoiding distortion)
    const auto p = std::get<PointLonLat>(projection->inv(PointXY{-rx / 2, ry / 2}));
    const auto q = std::get<PointLonLat>(projection->inv(PointXY{rx / 2, -ry / 2}));

    LongestElementDiagonal = 20. * util::Earth::distance({p.lon, p.lat}, {q.lon, q.lat});
    ASSERT(0. < LongestElementDiagonal);


    // bounding box
#if 1
    // [1] page 25, solution of s_d^2=0, restrained at x=0 (lon) and y=0 (lat). Note: uses a, b, height defined there
    auto eps_ll = 1e-6;

    auto n = 90. - util::radian_to_degree(0.151347) + eps_ll;
    auto s = -n;

    auto e = 90. - util::radian_to_degree(0.151853) + eps_ll + Lop;
    auto w = 2. * Lop - e;
#else
    auto geometric_maximum = [](double x_min, double x_eps, const std::function<double(double)>& f,
                                double f_eps = 1.e-9, size_t it_max = 1000) {
        size_t it = 0;
        auto x    = x_min;
        auto fx   = f(x);

        for (auto dx = x_eps; f_eps < dx && it < it_max; ++it) {
            auto fx_new = f(x + dx);
            if (!std::isfinite(fx_new) || fx_new < fx) {
                dx /= 2.;
            }
            else {
                x += dx;
                fx = fx_new;
                dx *= 2.;
            }
        }

        ASSERT(0. < fx && fx < 90.);
        return fx;
    };

    auto eps_xy = 1e-6 * h;
    auto eps_ll = 1e-6;

    auto max_lon = geometric_maximum(0., eps_xy, [&](double x) { return projectionGreenwich_.lonlat({x, 0}).lon(); });
    auto w       = Lop - max_lon - eps_ll;
    auto e       = Lop + max_lon + eps_ll;

    auto max_lat = geometric_maximum(0., eps_xy, [&](double y) { return projectionGreenwich_.lonlat({0, y}).lat(); });
    auto n       = max_lat + eps_ll;
    auto s       = -n;
#endif

    bbox = {n, w, s, e};

    x = LinearSpacing(xa_, xb_, Nx_, true);
    y = LinearSpacing(ya_, yb_, Ny_, true);
}


const std::vector<PointLonLat>& SpaceView::lonlat() const {
    if (lonlat_.empty()) {
        trace::Timer timer("SpaceView: pre-calculate (lon, lat) coordinates");

        ASSERT(projectionGreenwich_);  // Greenwich-centred (avoids PROJ normalisation)
        lonlat_.resize(x().size() * y().size());

        size_t index = 0;
        for (const auto& _y : y()) {
            for (const auto& _x : x()) {
                auto& ll = (lonlat_[index++] = {std::numeric_limits<double>::quiet_NaN(),
                                                std::numeric_limits<double>::quiet_NaN()});

                auto pll = std::get<PointLonLat>(projectionGreenwich_->inv(PointXY{_x, _y}));
                if (std::isfinite(pll.lon) && std::isfinite(pll.lat)) {
                    ASSERT(-90. < pll.lon && pll.lon < 90.);
                    ASSERT(-90. < pll.lat && pll.lat < 90.);

                    ll = {pll.lon + Lop, pll.lat};
                }
            }
        }
    }

    ASSERT(!lonlat_.empty());
    return lonlat_;
}


SpaceView::SpaceView(SpaceViewInternal&& internal) :
    RegularGrid(internal.projection.release(), internal.bbox, internal.x, internal.y, internal.shape),
    projectionGreenwich_(internal.projectionGreenwich.release()),
    LongestElementDiagonal(internal.LongestElementDiagonal),
    Lop(internal.Lop) {}


void SpaceView::fillGrib(grib_info& /*info*/) const {
    NOTIMP;
}


Iterator* SpaceView::iterator() const {
    class SpaceViewIterator : public Iterator {
        const std::vector<PointLonLat>& lonlat_;
        size_t count_;

        void print(std::ostream& out) const override {
            out << "SpaceViewIterator[";
            Iterator::print(out);
            out << ",count=" << count_ << "]";
        }

        bool next(Latitude& _lat, Longitude& _lon) override {
            while (count_ < lonlat_.size()) {
                // only one of (lon, lat) needs to be checked
                const auto& ll = lonlat_[count_++];
                if (std::isfinite(ll.lon)) {
                    _lat = lat(ll.lat);
                    _lon = lon(ll.lon);
                    return true;
                }
            }

            return false;
        }

        size_t index() const override { return count_; }

    public:
        SpaceViewIterator(const std::vector<PointLonLat>& lonlat) : lonlat_(lonlat), count_(0) {}
        ~SpaceViewIterator() override = default;

        SpaceViewIterator(const SpaceViewIterator&)            = delete;
        SpaceViewIterator(SpaceViewIterator&&)                 = delete;
        SpaceViewIterator& operator=(const SpaceViewIterator&) = delete;
        SpaceViewIterator& operator=(SpaceViewIterator&&)      = delete;
    };

    return new SpaceViewIterator(lonlat());
}


bool SpaceView::getLongestElementDiagonal(double& d) const {
    d = LongestElementDiagonal;
    return true;
}


}  // namespace mir::repres::regular
