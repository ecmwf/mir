// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <cmath>
#include <complex>
#include <sstream>
#include <vector>

#include "eckit/testing/Test.h"
#include "eckit/types/FloatCompare.h"

#include "mir/stats/detail/AngleT.h"
#include "mir/stats/detail/CentralMomentsT.h"
#include "mir/stats/detail/PNormsT.h"
#include "mir/stats/detail/ScalarT.h"

#include "stats.h"


namespace mir::stats::tests {


bool approx(double a, double b, double eps = 1e-12) {
    return eckit::types::is_approximately_equal(a, b, eps);
}


// mean 5, sum of squared deviations 32, of cubed deviations 42, of 4th powers 356
const std::vector<double> DATA{2., 4., 4., 4., 5., 5., 7., 9.};


CASE("CentralMomentsT") {
    using detail::CentralMomentsT;


    SECTION("mean, variance, standard deviation, skewness, kurtosis") {
        CentralMomentsT<double> stats;
        for (const auto& v : DATA) {
            stats(v);
        }

        EXPECT(approx(stats.mean(), 5.));
        EXPECT(approx(stats.variance(), 32. / 7.));
        EXPECT(approx(stats.standardDeviation(), std::sqrt(32. / 7.)));
        EXPECT(approx(stats.skewness(), 42. / 64.));
        EXPECT(approx(stats.kurtosis(), 356. * 8. / (32. * 32.) - 3.));

        EXPECT(approx(stats.centralMoment2(), 4.));
        EXPECT(approx(stats.centralMoment3(), 42. / 8.));
        EXPECT(approx(stats.centralMoment4(), 356. / 8.));

        EXPECT_EQUAL(stats.difference(2., -3.), 5.);

        std::ostringstream str;
        stats.print(str);
        EXPECT(contains(str.str(), "CentralMoments[mean=5,stddev=2.13809,skewness=0.65625,kurtosis=-0.21875,"));
    }


    SECTION("merge (combining moments of partitions)") {
        CentralMomentsT<double> all;
        CentralMomentsT<double> a;
        CentralMomentsT<double> b;
        CentralMomentsT<double> none;

        for (size_t i = 0; i < DATA.size(); ++i) {
            all(DATA[i]);
            (i < 3 ? a : b)(DATA[i]);
        }

        a += b;
        a += none;

        EXPECT(approx(a.mean(), all.mean()));
        EXPECT(approx(a.variance(), all.variance()));
        EXPECT(approx(a.skewness(), all.skewness()));
        EXPECT(approx(a.kurtosis(), all.kurtosis()));
        EXPECT(approx(a.centralMoment4(), all.centralMoment4()));

        none += all;
        EXPECT(approx(none.mean(), 5.));
        EXPECT(approx(none.variance(), 32. / 7.));
    }


    SECTION("empty, single value and reset") {
        CentralMomentsT<double> stats;

        EXPECT(std::isnan(stats.mean()));
        EXPECT_EQUAL(stats.variance(), 0.);
        EXPECT_EQUAL(stats.skewness(), 0.);
        EXPECT_EQUAL(stats.kurtosis(), 0.);
        EXPECT_EQUAL(stats.centralMoment1(), 0.);
        EXPECT_EQUAL(stats.centralMoment2(), 0.);
        EXPECT_EQUAL(stats.centralMoment3(), 0.);
        EXPECT_EQUAL(stats.centralMoment4(), 0.);

        stats(42.);
        EXPECT_EQUAL(stats.mean(), 42.);
        EXPECT_EQUAL(stats.variance(), 0.);
        EXPECT_EQUAL(stats.standardDeviation(), 0.);

        stats.reset();
        EXPECT(std::isnan(stats.mean()));
    }


    SECTION("complex (empty mean is NaN)") {
        CentralMomentsT<std::complex<double>> stats;
        EXPECT(std::isnan(stats.mean().real()) && std::isnan(stats.mean().imag()));

        stats({1., 2.});
        stats({3., -2.});
        EXPECT(stats.mean() == std::complex<double>(2., 0.));
    }
}


CASE("ScalarT") {
    detail::ScalarT<double> stats;
    for (const auto& v : DATA) {
        stats(v);
    }

    EXPECT(approx(stats.mean(), 5.));

    // print is virtual (overriding CentralMomentsT's)
    const detail::CentralMomentsT<double>& base = stats;
    std::ostringstream str;
    base.print(str);
    EXPECT_EQUAL(str.str(), "Scalar[mean=5,stddev=2.13809,skewness=0.65625,kurtosis=-0.21875]");
}


CASE("PNormsT") {
    detail::PNormsT<double> a;
    detail::PNormsT<double> b;

    a(3.);
    b(-4.);

    EXPECT_EQUAL(a.normLinfinity(), 3.);
    EXPECT_EQUAL(b.normLinfinity(), 4.);

    a += b;
    EXPECT_EQUAL(a.normL1(), 7.);
    EXPECT_EQUAL(a.normL2(), 5.);
    EXPECT_EQUAL(a.normLinfinity(), 4.);
    EXPECT_EQUAL(a.difference(-1., 2.), 3.);

    std::ostringstream str;
    a.print(str);
    EXPECT_EQUAL(str.str(), "PNorms[L1=7,L2=5,Li=4]");

    a.reset();
    EXPECT_EQUAL(a.normL1(), 0.);
    EXPECT_EQUAL(a.normL2(), 0.);
    EXPECT_EQUAL(a.normLinfinity(), 0.);
}


CASE("AngleT") {
    using detail::AngleScale;
    using detail::AngleSpace;
    using detail::AngleT;

    constexpr double eps = 1e-9;


    SECTION("degree, asymmetric [0, 360[") {
        AngleT<double, AngleScale::DEGREE, AngleSpace::ASYMMETRIC> stats;

        EXPECT(approx(stats.difference(350., 10.), 20.));
        EXPECT(approx(stats.difference(-10., 370.), 20.));
        EXPECT(approx(stats.difference(0., 180.), 180.));

        stats(350.);
        stats(10.);
        EXPECT(stats.difference(stats.mean(), 0.) < eps);

        stats.reset();
        stats(80.);
        stats(100.);
        EXPECT(approx(stats.mean(), 90., eps));

        stats.reset();
        stats(-90.);
        EXPECT(approx(stats.mean(), 270., eps));
    }


    SECTION("degree, symmetric [-180, 180[") {
        AngleT<double, AngleScale::DEGREE, AngleSpace::SYMMETRIC> stats;

        stats(170.);
        stats(-170.);
        EXPECT(stats.difference(stats.mean(), 180.) < eps);
        EXPECT(-180. <= stats.mean() && stats.mean() < 180.);

        stats.reset();
        stats(270.);
        EXPECT(approx(stats.mean(), -90., eps));

        // a single angle has no spread
        EXPECT_EQUAL(stats.variance(), 0.);
        EXPECT_EQUAL(stats.standardDeviation(), 0.);
    }


    SECTION("radian, asymmetric [0, 2π[") {
        AngleT<double, AngleScale::RADIAN, AngleSpace::ASYMMETRIC> a;
        AngleT<double, AngleScale::RADIAN, AngleSpace::ASYMMETRIC> b;

        a(-M_PI_2);
        b(-M_PI_2);
        a += b;
        EXPECT(approx(a.mean(), 3. * M_PI_2, eps));
        EXPECT(approx(a.difference(0.1, 2. * M_PI - 0.1), 0.2, eps));
    }


    SECTION("radian, symmetric [-π, π[") {
        AngleT<double, AngleScale::RADIAN, AngleSpace::SYMMETRIC> stats;

        stats(3. * M_PI_2);
        EXPECT(approx(stats.mean(), -M_PI_2, eps));

        std::ostringstream str;
        stats.print(str);
        EXPECT(contains(str.str(), "Angle[mean=-1.5708,stddev="));
    }
}


}  // namespace mir::stats::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
