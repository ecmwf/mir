// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

/*
 * Convolution kernels (distance weightings), on artificial data: a square stencil of neighbours around a point of the
 * Earth sphere, laid out on the local tangent plane (east, north), and fields defined from the offsets.
 *
 * Each kernel turns the neighbours into weights w_j (one matrix row); the filtered value is sum_j w_j f_j.
 */


#include <cmath>
#include <functional>
#include <limits>
#include <memory>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

#include "eckit/log/JSON.h"
#include "eckit/testing/Test.h"
#include "eckit/utils/MD5.h"

#include "mir/method/knn/distance/DistanceWeighting.h"
#include "mir/method/knn/distance/Kernel.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/util/CompensatedSum.h"
#include "mir/util/Exceptions.h"


namespace mir::tests::unit {


using method::knn::distance::DistanceWeighting;
using method::knn::distance::DistanceWeightingFactory;
using method::knn::distance::TangentPlane;
using neighbours_t = std::vector<search::PointSearch::PointValueType>;
using field_t      = std::function<double(double dx, double dy)>;


constexpr double RADIUS  = 6371229.;  // Earth radius [m]
constexpr double SPACING = 10000.;    // stencil spacing [m]
constexpr int HALF       = 6;         // stencil half-width (points), 13 x 13 neighbours
constexpr double EPS     = 1e-9;


/// A point on the sphere (latitude, longitude in degrees)
Point3 sphere(double lat, double lon) {
    const auto f = M_PI / 180.;
    return {RADIUS * std::cos(lat * f) * std::cos(lon * f), RADIUS * std::cos(lat * f) * std::sin(lon * f),
            RADIUS * std::sin(lat * f)};
}


/// Stencil around a point: neighbours at offsets (i, j) * SPACING on the tangent plane, the centre one first
struct Stencil {
    explicit Stencil(const Point3& p) : centre(p), plane(p) {
        add(0, 0);
        for (int j = -HALF; j <= HALF; ++j) {
            for (int i = -HALF; i <= HALF; ++i) {
                if (i != 0 || j != 0) {
                    add(i * SPACING, j * SPACING);
                }
            }
        }
    }

    void add(double dx, double dy) {
        const auto& e = plane.east;
        const auto& n = plane.north;
        const Point3 q{centre[0] + dx * e[0] + dy * n[0], centre[1] + dx * e[1] + dy * n[1],
                       centre[2] + dx * e[2] + dy * n[2]};
        neighbours.emplace_back(q, neighbours.size());
        offsets.emplace_back(dx, dy);
    }

    /// Weights (by neighbour index) of a kernel
    std::vector<double> weights(const DistanceWeighting& kernel) const {
        std::vector<method::WeightMatrix::Triplet> triplets;
        kernel(0, centre, neighbours, triplets);

        std::vector<double> w(neighbours.size(), 0.);
        for (const auto& t : triplets) {
            w.at(t.col()) += t.value();
        }
        return w;
    }

    /// Weights (by neighbour index) of a kernel, at any point
    std::vector<double> weights(const DistanceWeighting& kernel, const Point3& point) const {
        std::vector<method::WeightMatrix::Triplet> triplets;
        kernel(0, point, neighbours, triplets);

        std::vector<double> w(neighbours.size(), 0.);
        for (const auto& t : triplets) {
            w.at(t.col()) += t.value();
        }
        return w;
    }

    /// A point off the stencil, at offset (dx, dy) from the centre (on the tangent plane)
    Point3 at(double dx, double dy) const {
        const auto& e = plane.east;
        const auto& n = plane.north;
        return {centre[0] + dx * e[0] + dy * n[0], centre[1] + dx * e[1] + dy * n[1],
                centre[2] + dx * e[2] + dy * n[2]};
    }

    /// Interpolated value of a field at any point: sum_j w_j f_j
    double apply(const DistanceWeighting& kernel, const field_t& f, const Point3& point) const {
        const auto w = weights(kernel, point);
        double sum   = 0.;
        for (size_t j = 0; j < w.size(); ++j) {
            sum += w[j] * f(offsets[j].first, offsets[j].second);
        }
        return sum;
    }

    /// Filtered value of a field at the centre: sum_j w_j f_j
    double apply(const DistanceWeighting& kernel, const field_t& f) const {
        const auto w = weights(kernel);
        double sum   = 0.;
        for (size_t j = 0; j < w.size(); ++j) {
            sum += w[j] * f(offsets[j].first, offsets[j].second);
        }
        return sum;
    }

    Point3 centre;
    TangentPlane plane;
    neighbours_t neighbours;
    std::vector<std::pair<double, double>> offsets;
};


std::unique_ptr<const DistanceWeighting> build(const std::string& name, const param::SimpleParametrisation& param) {
    return std::unique_ptr<const DistanceWeighting>(DistanceWeightingFactory::build(name, param));
}


double sum(const std::vector<double>& w) {
    double s = 0.;
    for (const auto& v : w) {
        s += v;
    }
    return s;
}


bool approx(double a, double b, double eps = EPS) {
    return std::abs(a - b) <= eps * std::max(1., std::abs(b));
}


const Stencil STENCIL(sphere(45., 30.));


/**
 * Gradient, on the tangent plane at another point, of the linear field gx dx + gy dy (of the stencil offsets): the
 * planes are slightly rotated (the meridians converge), the gradient is A^-T (gx, gy), with A the map between them
 */
std::pair<double, double> gradient_at(const Point3& point, double gx, double gy) {
    auto dot = [](const Point3& a, const Point3& b) { return a[0] * b[0] + a[1] * b[1] + a[2] * b[2]; };
    const TangentPlane plane(point);
    const auto a = dot(STENCIL.plane.east, plane.east);
    const auto b = dot(STENCIL.plane.north, plane.east);
    const auto c = dot(STENCIL.plane.east, plane.north);
    const auto d = dot(STENCIL.plane.north, plane.north);

    const auto det = a * d - b * c;
    return {(d * gx - c * gy) / det, (-b * gx + a * gy) / det};
}


CASE("radial smoothing kernels") {
    // kernel radius 40 km: the compact kernels reach 4 points in each direction
    param::SimpleParametrisation param;
    param.set("kernel-radius", 40000.);
    param.set("distance-weighting-gaussian-stddev", 20000.);

    SECTION("weights are positive, add up to one, and preserve constant and linear fields") {
        for (const auto* name : {"gaussian", "top-hat", "hann", "epanechnikov"}) {
            auto kernel  = build(name, param);
            const auto w = STENCIL.weights(*kernel);

            for (const auto& v : w) {
                EXPECT(v >= 0.);
            }
            EXPECT(approx(sum(w), 1.));

            // symmetric kernels: a linear field is unchanged at the centre
            EXPECT(approx(STENCIL.apply(*kernel, [](double, double) { return 7.; }), 7.));
            EXPECT(approx(STENCIL.apply(*kernel, [](double dx, double dy) { return 1. + dx - 2. * dy; }), 1.));

            // a checkerboard is smoothed out
            const field_t checkerboard = [](double dx, double dy) {
                return (static_cast<long>(std::round((dx + dy) / SPACING)) % 2 == 0) ? 1. : -1.;
            };
            EXPECT(std::abs(STENCIL.apply(*kernel, checkerboard)) < 0.5);
        }
    }

    SECTION("top-hat: plain average inside the radius") {
        auto kernel  = build("top-hat", param);
        const auto w = STENCIL.weights(*kernel);

        // all points used have the same weight, and are within 40 km (or on the circle)
        size_t used = 0;
        for (size_t j = 0; j < w.size(); ++j) {
            if (w[j] > 0.) {
                ++used;
                EXPECT(std::hypot(STENCIL.offsets[j].first, STENCIL.offsets[j].second) <= 40000. + 1.);
            }
        }
        for (const auto& v : w) {
            EXPECT(v == 0. || approx(v, 1. / static_cast<double>(used)));
        }
        EXPECT(used >= 45);  // the 45 stencil points strictly inside the circle, at least
    }

    SECTION("gaussian: weights are exp(-r^2 / 2 sigma^2), normalised") {
        const auto w     = STENCIL.weights(*build("gaussian", param));
        const auto ratio = w[1] / w[0];  // first neighbour: (-6, -6) spacings away from the centre
        const auto r2    = 2. * std::pow(HALF * SPACING, 2);
        EXPECT(approx(ratio, std::exp(-r2 / (2. * 20000. * 20000.)), 1e-6));
    }
}


CASE("directional kernel") {
    // elliptical gaussian: 30 km along the kernel direction, 5 km across
    param::SimpleParametrisation param;
    param.set("kernel", "gaussian");
    param.set("kernel-radius", 30000.);
    param.set("kernel-radius-across", 5000.);

    // a field varying only from west to east (wavelength 40 km)
    const field_t waves = [](double dx, double) { return std::cos(2. * M_PI * dx / 40000.); };

    SECTION("east-west kernel (angle 0): smooths east-west variations") {
        param.set("kernel-angle", 0.);
        auto kernel = build("directional", param);

        EXPECT(approx(sum(STENCIL.weights(*kernel)), 1.));
        EXPECT(std::abs(STENCIL.apply(*kernel, waves)) < 0.1);
    }

    SECTION("north-south kernel (angle 90): keeps most east-west variations") {
        param.set("kernel-angle", 90.);
        auto kernel = build("directional", param);

        // the neighbours 10 km east and west (2 radii across) still weigh exp(-2) of the centre
        EXPECT(STENCIL.apply(*kernel, waves) > 0.75);
    }

    SECTION("other profiles") {
        for (const auto* name : {"top-hat", "hann", "epanechnikov"}) {
            param.set("kernel", name);
            param.set("kernel-angle", 0.);
            EXPECT(approx(sum(STENCIL.weights(*build("directional", param))), 1.));
        }
    }
}


CASE("signed kernels: derivative") {
    // weighted least-squares plane fit, gaussian weights (radius 20 km): exact for linear fields
    param::SimpleParametrisation param;
    param.set("kernel-radius", 20000.);

    const field_t linear = [](double dx, double dy) { return 3. + 2e-3 * dx - 5e-3 * dy; };  // per metre

    for (const auto& [angle, expected] :
         std::vector<std::pair<double, double>>{{0., 2e-3}, {90., -5e-3}, {45., (2e-3 - 5e-3) / std::sqrt(2.)}}) {
        param.set("kernel-angle", angle);
        auto kernel = build("derivative", param);

        EXPECT(std::abs(sum(STENCIL.weights(*kernel))) < 1e-12);  // zero-sum
        EXPECT(approx(STENCIL.apply(*kernel, linear), expected, 1e-8));
    }

    SECTION("Sobel-like edge detection: the derivative across a smooth step") {
        param.set("kernel-angle", 0.);
        const field_t step = [](double dx, double) { return std::tanh(dx / 20000.); };  // slope 1/20000 at the centre
        auto kernel        = build("derivative", param);
        EXPECT(approx(STENCIL.apply(*kernel, step), 1. / 20000., 0.1));
    }

    SECTION("not enough neighbours") {
        const Stencil line(sphere(0., 0.));
        neighbours_t two(line.neighbours.begin(), line.neighbours.begin() + 2);
        std::vector<method::WeightMatrix::Triplet> triplets;
        EXPECT_THROWS_AS((*build("derivative", param))(0, line.centre, two, triplets), exception::UserError);
    }
}


CASE("signed kernels: laplacian") {
    // weighted least-squares quadratic fit, gaussian weights (radius 20 km): exact for quadratic fields
    param::SimpleParametrisation param;
    param.set("kernel-radius", 20000.);
    auto kernel = build("laplacian", param);

    EXPECT(std::abs(sum(STENCIL.weights(*kernel))) < 1e-15);  // zero-sum (units: per square metre)

    EXPECT(std::abs(STENCIL.apply(*kernel, [](double, double) { return 1.; })) < 1e-15);
    EXPECT(approx(STENCIL.apply(*kernel, [](double dx, double dy) { return dx * dx + dy * dy; }), 4., 1e-8));
    EXPECT(std::abs(STENCIL.apply(*kernel, [](double dx, double dy) { return dx * dx - dy * dy + dx * dy; })) < 1e-8);

    SECTION("other profiles") {
        for (const auto* name : {"top-hat", "epanechnikov"}) {
            param.set("kernel", name);
            param.set("kernel-radius", 50000.);
            EXPECT(approx(STENCIL.apply(*build("laplacian", param),
                                        [](double dx, double dy) { return 3. * dx * dx - dy * dy + dx; }),
                          4., 1e-8));
        }
    }
}


CASE("signed kernels: difference of gaussians") {
    // band-pass (edge detection): gaussian of 10 km minus gaussian of 16 km (the default ratio 1.6)
    param::SimpleParametrisation param;
    param.set("kernel-radius", 10000.);
    auto kernel = build("difference-of-gaussians", param);

    EXPECT(std::abs(sum(STENCIL.weights(*kernel))) < 1e-12);
    EXPECT(std::abs(STENCIL.apply(*kernel, [](double, double) { return 5.; })) < 1e-12);

    // a peak at the centre stands out (positive), a constant background does not
    EXPECT(STENCIL.apply(*kernel, [](double dx, double dy) { return dx == 0. && dy == 0. ? 1. : 0.; }) > 0.);

    param.set("kernel-radius-outer", 5000.);
    EXPECT_THROWS_AS(build("difference-of-gaussians", param), exception::UserError);
}


CASE("kernel profiles") {
    using method::knn::distance::KernelProfile;

    // K(s) at s = 0, 0.5, 1 and beyond (s = r / h)
    for (const auto& [name, k0, k05, k1] : std::vector<std::tuple<std::string, double, double, double>>{
             {"gaussian", 1., std::exp(-0.125), std::exp(-0.5)},
             {"top-hat", 1., 1., 1.},
             {"hann", 1., 0.5, 0.},
             {"epanechnikov", 1., 0.75, 0.},
         }) {
        const KernelProfile K(name);
        EXPECT(K.name() == name);
        EXPECT(approx(K(0.), k0));
        EXPECT(approx(K(0.5), k05));
        EXPECT(approx(K(1.), k1));
        EXPECT(name == "gaussian" ? K(2.) > 0. : K(2.) == 0.);

        // from the squared distance, as the weightings use them
        EXPECT(approx(K.at_distance2(0.25 * 4., 2.), k05));
    }

    SECTION("same profiles") {
        EXPECT(KernelProfile("hann").sameAs(KernelProfile("hann")));
        EXPECT(!KernelProfile("hann").sameAs(KernelProfile("epanechnikov")));
    }

    SECTION("errors") {
        EXPECT_THROWS_AS(KernelProfile("unknown"), exception::UserError);
    }
}


CASE("tangent plane") {
    // orthonormal axes, east and north, anywhere (including the poles, where east is the y axis)
    auto dot = [](const Point3& a, const Point3& b) { return a[0] * b[0] + a[1] * b[1] + a[2] * b[2]; };

    for (const auto& [lat, lon] : std::vector<std::pair<double, double>>{{45., 30.}, {0., 0.}, {90., 0.}, {-90., 0.}}) {
        const TangentPlane plane(sphere(lat, lon));
        EXPECT(approx(dot(plane.east, plane.east), 1.));
        EXPECT(approx(dot(plane.north, plane.north), 1.));
        EXPECT(std::abs(dot(plane.east, plane.north)) < EPS);
        EXPECT(std::abs(dot(plane.east, plane.origin)) < EPS * RADIUS);
    }

    // north points to the north pole
    const TangentPlane plane(sphere(0., 0.));
    EXPECT(approx(plane.north[2], 1.));
    EXPECT(approx(plane.east[1], 1.));

    // offsets are projections on the axes
    const auto [dx, dy] = plane(Point3{RADIUS, 3., 4.});
    EXPECT(approx(dx, 3.));
    EXPECT(approx(dy, 4.));

    SECTION("derivative at the poles") {
        param::SimpleParametrisation param;
        param.set("kernel-radius", 20000.);
        param.set("kernel-angle", 90.);

        const Stencil pole(sphere(90., 0.));
        EXPECT(approx(pole.apply(*build("derivative", param), [](double dx, double dy) { return dx - 3. * dy; }), -3.,
                      1e-8));
    }
}


CASE("kernels interpolate (k-nearest, between input points)") {
    // an output point off the stencil, 3 km east and 4 km north of the centre
    const auto point = STENCIL.at(3000., 4000.);

    param::SimpleParametrisation param;
    param.set("kernel-radius", 20000.);

    SECTION("smoothing kernels: weights add up to one, constants are kept") {
        param.set("kernel-radius-across", 10000.);
        for (const auto* name : {"gaussian", "top-hat", "hann", "epanechnikov", "directional"}) {
            auto kernel = build(name, param);
            EXPECT(approx(sum(STENCIL.weights(*kernel, point)), 1.));
            EXPECT(approx(STENCIL.apply(*kernel, [](double, double) { return 3.; }, point), 3.));
        }
    }

    SECTION("derivative and laplacian at the output point") {
        // the linear field 2 dx + dy, on the output point tangent plane
        const auto [east, north] = gradient_at(point, 2., 1.);
        EXPECT(std::abs(east - 2.) > 1e-6);  // the rotation is noticeable

        const field_t linear = [](double dx, double dy) { return 2. * dx + dy; };
        param.set("kernel-angle", 0.);
        EXPECT(approx(STENCIL.apply(*build("derivative", param), linear, point), east, 1e-8));
        param.set("kernel-angle", 90.);
        EXPECT(approx(STENCIL.apply(*build("derivative", param), linear, point), north, 1e-8));

        // quadratic field (dx - 3000)^2 + (dy - 4000)^2, with its minimum at the output point: value 0, laplacian 4
        const field_t bowl = [](double dx, double dy) { return std::pow(dx - 3000., 2) + std::pow(dy - 4000., 2); };
        EXPECT(approx(STENCIL.apply(*build("laplacian", param), bowl, point), 4., 1e-5));
    }

    SECTION("derivative, extrapolated (an output point outside the stencil, 80 km east of its centre)") {
        const auto outside       = STENCIL.at(80000., 0.);
        const auto [east, north] = gradient_at(outside, 2., 1.);

        const field_t linear = [](double dx, double dy) { return 2. * dx + dy; };
        param.set("kernel-angle", 0.);
        EXPECT(approx(STENCIL.apply(*build("derivative", param), linear, outside), east, 1e-8));
        param.set("kernel-angle", 90.);
        EXPECT(approx(STENCIL.apply(*build("derivative", param), linear, outside), north, 1e-8));
    }

    SECTION("no neighbours inside a compact kernel") {
        param.set("kernel", "top-hat");
        param.set("kernel-radius", 1000.);

        std::vector<method::WeightMatrix::Triplet> triplets;
        EXPECT_THROWS_AS((*build("derivative", param))(0, point, STENCIL.neighbours, triplets), exception::UserError);
    }
}


CASE("nearest neighbour, if no neighbours are inside a compact kernel") {
    // a stencil without its centre, and a kernel smaller than the spacing
    neighbours_t ring(STENCIL.neighbours.begin() + 1, STENCIL.neighbours.end());

    param::SimpleParametrisation param;
    param.set("kernel-radius", 1000.);

    for (const auto* name : {"top-hat", "hann", "epanechnikov", "directional"}) {
        param.set("kernel", "top-hat");
        std::vector<method::WeightMatrix::Triplet> triplets;
        (*build(name, param))(0, STENCIL.centre, ring, triplets);

        EXPECT(triplets.size() == 1);
        EXPECT(triplets.front().col() == ring.front().payload());
        EXPECT(triplets.front().value() == 1.);
    }
}


CASE("json, print, sameAs and hash") {
    param::SimpleParametrisation a;
    a.set("kernel-radius", 20000.);
    a.set("kernel-radius-across", 5000.);
    a.set("kernel-radius-outer", 40000.);
    a.set("kernel-angle", 30.);
    a.set("distance-weighting-gaussian-stddev", 20000.);

    param::SimpleParametrisation b;  // different radius
    b.set("kernel-radius", 25000.);
    b.set("kernel-radius-across", 5000.);
    b.set("kernel-radius-outer", 40000.);
    b.set("kernel-angle", 30.);
    b.set("distance-weighting-gaussian-stddev", 25000.);

    auto json = [](const DistanceWeighting& d) {
        std::ostringstream s;
        eckit::JSON j(s);
        j.startObject();
        d.json(j);
        j.endObject();
        return s.str();
    };

    auto digest = [](const DistanceWeighting& d) {
        eckit::MD5 h;
        d.hash(h);
        return h.digest();
    };

    for (const auto* name : {"gaussian", "top-hat", "hann", "epanechnikov", "directional", "derivative", "laplacian",
                             "difference-of-gaussians"}) {
        auto a1 = build(name, a);
        auto a2 = build(name, a);
        auto b1 = build(name, b);

        EXPECT(a1->sameAs(*a2));
        EXPECT(!a1->sameAs(*b1));
        EXPECT(digest(*a1) == digest(*a2));
        EXPECT(digest(*a1) != digest(*b1));

        // json names the weighting (and its options), print too
        EXPECT(json(*a1).find(std::string("\"distance-weighting\":\"") + name + "\"") != std::string::npos);

        std::ostringstream s;
        s << *a1;
        EXPECT(!s.str().empty());
    }

    // kernels with the same radius, but different types, are not the same
    EXPECT(!build("top-hat", a)->sameAs(*build("hann", a)));
    EXPECT(!build("derivative", a)->sameAs(*build("laplacian", a)));

    SECTION("gaussian is unchanged (for the cached matrices)") {
        auto g = build("gaussian", a);
        EXPECT(json(*g) == R"({"distance-weighting":"gaussian","distance-weighting-gaussian-stddev":20000})");

        std::ostringstream s;
        s << *g;
        EXPECT(s.str() == "GaussianDistanceWeighting[stddev=20000]");
    }
}


CASE("errors") {
    param::SimpleParametrisation param;

    SECTION("non-positive radii") {
        param.set("kernel-radius", 0.);
        for (const auto* name : {"top-hat", "directional", "derivative", "laplacian", "difference-of-gaussians"}) {
            EXPECT_THROWS_AS(build(name, param), exception::UserError);
        }
    }

    SECTION("directional: across radius, unknown profile") {
        param.set("kernel-radius", 1000.);
        param.set("kernel-radius-across", -1.);
        EXPECT_THROWS_AS(build("directional", param), exception::UserError);

        param.set("kernel-radius-across", 1000.);
        param.set("kernel", "unknown");
        EXPECT_THROWS_AS(build("directional", param), exception::UserError);
    }
}


CASE("many neighbours: smoothing weights still add up to one") {
    // e.g. near the poles of a regular lat-lon grid, a few thousand neighbours (a naive sum drifts by ~n epsilon)
    const double eps = 1e2 * std::numeric_limits<double>::epsilon();  // as WeightMatrix::validate

    util::CompensatedSum<double> compensated;
    double naive = 0.;
    for (size_t i = 0; i < 1000000; ++i) {
        compensated += 0.1;
        naive += 0.1;
    }
    EXPECT(std::abs(compensated.value() - 100000.) < 1e-9);
    EXPECT(std::abs(naive - 100000.) > 1e-9);

    // 5000 neighbours on a ring of 1 km around the centre
    neighbours_t ring;
    for (size_t i = 0; i < 5000; ++i) {
        const auto a = 2. * M_PI * static_cast<double>(i) / 5000.;
        ring.emplace_back(STENCIL.at(1000. * std::cos(a), 1000. * std::sin(a)), i);
    }

    param::SimpleParametrisation param;
    param.set("kernel-radius", 2000.);
    param.set("distance-weighting-gaussian-stddev", 2000.);
    param.set("kernel-radius-across", 1500.);

    for (const auto* name : {"top-hat", "hann", "gaussian", "directional"}) {
        std::vector<method::WeightMatrix::Triplet> triplets;
        (*build(name, param))(0, STENCIL.centre, ring, triplets);
        EXPECT(triplets.size() == ring.size());

        util::CompensatedSum<double> sum;
        for (const auto& t : triplets) {
            sum += t.value();
        }
        EXPECT(std::abs(sum.value() - 1.) < eps);
    }
}


CASE("signed kernels are not validated nor pruned as interpolation weights") {
    param::SimpleParametrisation param;
    param.set("kernel-radius", 20000.);

    for (const auto* name : {"derivative", "laplacian", "difference-of-gaussians"}) {
        auto kernel = build(name, param);
        EXPECT(!kernel->validateMatrixWeights().sum);
        EXPECT(!kernel->validateMatrixWeights().bounds);
        EXPECT(!kernel->pruneMatrixWeights());
    }

    for (const auto* name : {"gaussian", "top-hat", "directional"}) {
        auto kernel = build(name, param);
        EXPECT(kernel->validateMatrixWeights().sum);
        EXPECT(kernel->pruneMatrixWeights());
    }
}


}  // namespace mir::tests::unit


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
