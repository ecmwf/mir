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


#include <cmath>
#include <functional>
#include <limits>
#include <memory>
#include <random>
#include <sstream>

#include "eckit/linalg/Matrix.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/types/FloatCompare.h"

#include "mir/data/MIRField.h"
#include "mir/input/GribFileInput.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"
#include "mir/tools/MIRTool.h"
#include "mir/util/Angles.h"
#include "mir/util/Atlas.h"
#include "mir/util/BoundingBox.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"


namespace mir {
namespace tools {


double bisection_method(double x_min, double x_max, const std::function<double(double)>& f, double f0 = 0.,
                        double f_eps = 1.e-9) {
    static std::random_device rd;
    static const auto entropy = rd();
    std::mt19937 gen(entropy);
    std::uniform_real_distribution<double> dis(x_min, x_max);

    auto xp = dis(gen);
    while (f(xp) < f0) {
        xp = dis(gen);
    }

    auto xm = dis(gen);
    while (f(xm) > f0) {
        xm = dis(gen);
    }

    auto x = (xp + xm) / 2.;
    for (auto fx = f(x) - f0; !eckit::types::is_approximately_equal(fx, 0., f_eps);
         x = (xp + xm) / 2., fx = f(x) - f0) {
        (fx >= 0. ? xp : xm) = x;
    }

    return x;
}


double geometric_maximum(double x_min, const std::function<double(double)>& f, double f_eps = 1.e-9) {
    if (std::isinf(f(x_min)) != 0) {
        return x_min;
    }

    size_t it = 0;
    auto x    = x_min;
    for (auto dx = 1., fx = f(x); f_eps < dx; ++it) {
        auto fx_new = f(x + dx);
        if (std::isinf(fx_new) != 0 || fx_new < fx) {
            dx /= 2.;
        }
        else {
            x += dx;
            fx = fx_new;
            dx *= 2.;
        }
    }

    Log::info() << "it = " << it << std::endl;
    return x;
}


// distance from Earth centre to satellite/equator/pole(s)
const double SAT_HEIGHT = 42164.;
const double R_EQ       = 6378.169;
const double R_POL      = 6356.5838;
const double RR         = (R_POL * R_POL) / (R_EQ * R_EQ);


Point2 forward_projection(const PointLatLon& ll) {
    auto lat = ll.lat();
    auto lon = ll.lon();
    ASSERT(Latitude::SOUTH_POLE <= lat && lat <= Latitude::NORTH_POLE);
    ASSERT(Longitude::MINUS_DATE_LINE <= lon && lon <= Longitude::DATE_LINE);

    double latr = util::degree_to_radian(lat.value());
    double lonr = util::degree_to_radian(lon.value());

    double c_lat = std::atan(RR * std::tan(latr));  //< geocentric latitude
    double re    = R_POL / std::sqrt(1. - (1. - RR) * std::cos(c_lat) * std::cos(c_lat));

    double rl = re;
    double r1 = SAT_HEIGHT - rl * std::cos(c_lat) * std::cos(lonr);
    double r2 = -rl * std::cos(c_lat) * std::sin(lonr);
    double r3 = rl * std::sin(c_lat);
    double rn = std::sqrt(r1 * r1 + r2 * r2 + r3 * r3);

    // visibility checks for positive dot product between (point, satellite) . (point, Earth centre)
    auto dot = r1 * rl * std::cos(c_lat) * std::cos(lonr) - r2 * r2 - r3 * r3 / RR;
    if (dot <= 0) {
        return {std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity()};
    }

    return {std::atan(-r2 / r1), std::asin(-r3 / rn)};
}


PointLatLon inverse_projection(const Point2& xy) {
    auto x = xy.x();
    auto y = xy.y();
    ASSERT(std::isinf(x) == 0);
    ASSERT(std::isinf(y) == 0);

    double f1 = SAT_HEIGHT * std::cos(x) * std::cos(y);
    double f2 = std::cos(y) * std::cos(y) + std::sin(y) * std::sin(y) / RR;

    double sd = std::sqrt(f1 * f1 - f2 * 1737121856.);
    double sn = (f1 - sd) / f2;

    double s1  = SAT_HEIGHT - sn * std::cos(x) * std::cos(y);
    double s2  = sn * std::sin(x) * std::cos(y);
    double s3  = -sn * std::sin(y);
    double sxy = std::sqrt(s1 * s1 + s2 * s2);

    auto lon = util::radian_to_degree(std::atan(s2 / s1));
    auto lat = util::radian_to_degree(std::atan(s3 / (sxy * RR)));
    return {lat, lon};
}


template <typename EXTERNAL_T, typename INTERNAL_T = EXTERNAL_T>
EXTERNAL_T get(const param::MIRParametrisation& param, const std::string& key) {
    INTERNAL_T value;
    ASSERT(param.get(key, value));
    return static_cast<EXTERNAL_T>(value);
}


void interpret(const param::MIRParametrisation& param, const data::MIRField& field) {
    // References:
    // - LRIT/HRIT Global Specification (CGMS 03, Issue 2.6, 12.08.1999)
    // - MSG Ground Segment LRIT/HRIT Mission Specific Implementation, EUMETSAT Document, (EUM/MSG/SPE/057, Issue 6, 21.
    // June 2006).

    auto Nr = get<double>(param, "NrInRadiusOfEarth") * (get<long>(param, "edition") == 1 ? 1e-6 : 1.);
    ASSERT(Nr > 1.);
    auto height = (Nr - 1.) * R_EQ;

    auto dx = get<double, long>(param, "dx");
    auto dy = get<double, long>(param, "dy");
    auto xp = get<double, long>(param, "XpInGridLengths");
    auto yp = get<double, long>(param, "YpInGridLengths");

    auto Nc = get<int>(param, "numberOfPointsAlongXAxis");
    auto Nl = get<int>(param, "numberOfPointsAlongYAxis");
    ASSERT(0 < Nc);
    ASSERT(0 < Nl);

    auto slat = get<double>(param, "latitudeOfSubSatellitePointInDegrees");
    auto slon = get<double>(param, "longitudeOfSubSatellitePointInDegrees");
    ASSERT(eckit::types::is_approximately_equal(slat, 0.));

    double east  = 180.;
    double south = -90.;
    double dlon  = 0.1;
    double dlat  = -0.1;

    size_t Ni = 3600;
    size_t Nj = 1801;
    ASSERT(Ni > 1);
    ASSERT(Nj > 1);


    // field values
    ASSERT(field.dimensions() == 1);
    auto& values      = field.values(0);
    auto missingValue = field.missingValue();


#if 1
    atlas::Projection::Spec spec("type", "proj");

    std::ostringstream str;
    str << " +proj=geos";
    str << " +h=" << height;
    str << " +lon_0=" << 0.;
    str << " +sweep=y";
    str << " +a=" << R_EQ;
    str << " +b=" << R_POL;
    str << " +type=crs";

    Log::info() << "proj = " << str.str() << std::endl;

    spec.set("proj", str.str());
    atlas::Projection proj(spec);
#endif


    auto lat_max_x = geometric_maximum(1e-3, [&proj](const double x) { return proj.lonlat({0, x}).lat(); });
    auto lon_max_x = geometric_maximum(1e-3, [&proj](const double x) { return proj.lonlat({x, 0}).lon(); });

    auto eps = 1e-9;
    auto n   = proj.lonlat({0, lat_max_x}).lat() + eps;
    auto s   = proj.lonlat({0, -lat_max_x}).lat() - eps;
    auto w   = proj.lonlat({-lon_max_x, 0}).lon() - eps;
    auto e   = proj.lonlat({lon_max_x, 0}).lon() + eps;
    util::BoundingBox bbox(n, w, s, e);
    Log::info() << bbox << std::endl;

    // scaling coefficients
    auto angularSize = 2. * std::asin(1. / Nr);  //< apparent angular size of the Earth
    double cfac      = -dx / angularSize;
    double lfac      = -dy / angularSize;

#if 0
    eckit::linalg::Matrix matrix(Nj, Ni);
    auto matrix_it = matrix.begin();

    for (size_t i = 1900; i < Ni; ++i) {
        for (size_t j = 1000; j < Nj; ++j, ++matrix_it) {
            auto lat = south - double(j) * dlat;
            auto lon = east - double(i) * dlon;

            Log::info() << "lo = " << lon << "\nla = " << lat << std::endl;
            auto xy1 = forward_projection({lat, lon + slon});  // should be (lon - slond)?
            Log::info() << "x1 = " << xy1.x() << "\ny1 = " << xy1.y() << std::endl;
            auto ll1 = inverse_projection(xy1);
            Log::info() << "lo = " << ll1.lon() << "\nla = " << ll1.lat() << std::endl;
            Log::info() << "." << std::endl;
            auto xy2 = proj.xy({lon + slon, lat});
            Log::info() << "x2 = " << xy2.x() << "\ny2 = " << xy2.y() << std::endl;
            auto ll2 = proj.lonlat(xy2);
            Log::info() << "lo = " << ll2.lon() << "\nla = " << ll2.lat() << std::endl;
            Log::info() << "." << std::endl;

            // convert to pixel column and row using the scaling functions
            auto c = static_cast<int>(std::lround(xp + xy1.x() * cfac));
            auto l = static_cast<int>(std::lround(yp + xy1.y() * lfac));

            *matrix_it = (0 <= c && c < Nc && 0 <= l && l < Nl) ? values[size_t(l * Nc + c)] : missingValue;
        }
    }
#endif
}


struct MIRSpaceView : MIRTool {
    using MIRTool::MIRTool;

    int numberOfPositionalArguments() const override { return 1; }

    void usage(const std::string& tool) const override {
        Log::info() << "\nUsage: " << tool << " file.grib" << std::endl;
    }

    void execute(const eckit::option::CmdArgs&) override;
};


void MIRSpaceView::execute(const eckit::option::CmdArgs& args) {
    ASSERT(args.count() == 1);

    std::unique_ptr<input::MIRInput> input(new input::GribFileInput(args(0)));
    ASSERT(input->next());

    auto field = input->field();
    // auto missingValue  = field.missingValue();
    // const auto& values = field.values(0);

    repres::RepresentationHandle rep(field.representation());
    // for (std::unique_ptr<repres::Iterator> it(rep->iterator()); it->next();) {
    // }

    auto grid  = rep->atlasGrid();
    auto& proj = grid.projection();

    auto& log = Log::info();
    log.precision(32);

    log << proj.spec() << std::endl;

    auto lat = [&proj](const double x) { return proj.lonlat({0, x}).lat(); };
    auto lon = [&proj](const double x) { return proj.lonlat({x, 0}).lon(); };
    auto x   = [&proj](const double x) { return proj.xy({x, 0}).x(); };
    auto y   = [&proj](const double x) { return proj.xy({0, x}).y(); };

    auto m = geometric_maximum(1, lat);
    log << "f(" << m << ")=" << lat(m) << " [lat max]\nr=" << y(lat(m)) << std::endl;

    m = geometric_maximum(1, lon);
    log << "f(" << m << ")=" << lon(m) << " [lon max]\nr=" << x(lon(m)) << std::endl;

    interpret(input->parametrisation(), field);

#if 0
    auto r = bisection_method(0, 1e7, lat, 81.3, 1e-6);
    log << "f(" << r << ")=" << lat(r) << " [N]\nr=" << y(lat(r)) << std::endl;

    r = bisection_method(-1e7, 0, lat, -81.3, 1e-6);
    log << "f(" << r << ")=" << lat(r) << " [S]\nr=" << y(lat(r)) << std::endl;

    r = bisection_method(-1e7, 0, lon, -81.242, 1e-6);
    log << "f(" << r << ")=" << lon(r) << " [W]\nr=" << x(lon(r)) << std::endl;

    r = bisection_method(0, 1e7, lon, 81.242, 1e-6);
    log << "f(" << r << ")=" << lon(r) << " [E]\nr=" << x(lon(r)) << std::endl;
#endif
}


}  // namespace tools
}  // namespace mir


int main(int argc, char** argv) {
    mir::tools::MIRSpaceView tool(argc, argv);
    return tool.start();
}
