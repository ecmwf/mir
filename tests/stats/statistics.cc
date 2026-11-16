// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <cmath>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "eckit/exception/Exceptions.h"
#include "eckit/testing/Test.h"
#include "eckit/types/FloatCompare.h"

#include "mir/param/SimpleParametrisation.h"
#include "mir/stats/Statistics.h"
#include "mir/stats/statistics/GridBoxIntegral.h"
#include "mir/stats/statistics/Integral.h"
#include "mir/stats/statistics/SimplePackingEntropy.h"
#include "mir/stats/statistics/Spectral.h"
#include "mir/util/Atlas.h"
#include "mir/util/Log.h"

#include "stats.h"


namespace mir::stats::tests {


/// Statistics (by name) of the field, as printed
std::string execute(const std::string& name, const data::MIRField& field) {
    param::SimpleParametrisation param;
    std::unique_ptr<Statistics> stats(StatisticsFactory::build(name, param));

    stats->execute(field);
    return to_string(*stats);
}


CASE("StatisticsT") {
    // mean 5, sum of squared deviations 32, of cubed deviations 42, of 4th powers 356 (12 points)
    const auto field =
        make_field("{grid: [90, 90]}", {MISSING, 2., 4., 4., 4., MISSING, 5., 5., 7., 9., MISSING, MISSING});
    EXPECT(field.hasMissing());

    const std::string counter = "Statistics[Counter[count=12,missing=4,max=9,maxIndex=9,min=2,minIndex=1]";


    SECTION("min-max") {
        EXPECT_EQUAL(execute("min-max", field), counter + "]");
    }


    SECTION("scalar") {
        EXPECT_EQUAL(execute("scalar", field),
                     counter + ",Scalar[mean=5,stddev=2.13809,skewness=0.65625,kurtosis=-0.21875]]");
    }


    SECTION("central-moments") {
        auto str = execute("central-moments", field);
        EXPECT(contains(str, counter + ",CentralMoments[mean=5,stddev=2.13809,skewness=0.65625,kurtosis=-0.21875,"));
        EXPECT(contains(str, ",centralMoment2=4,centralMoment3=5.25,centralMoment4=44.5]]"));
    }


    SECTION("p-norms") {
        EXPECT_EQUAL(execute("p-norms", field), counter + ",PNorms[L1=40,L2=15.2315,Li=9]]");
    }


    SECTION("angle-*") {
        const auto degrees = make_field("{grid: [90, 90]}", [](double lat, double) {
            return lat > 0 ? MISSING : lat < 0 ? -90. : 270.;
        });

        EXPECT(contains(execute("angle-degree-asymmetric", degrees), "Angle[mean=270,"));
        EXPECT(contains(execute("angle-degree-symmetric", degrees), "Angle[mean=-90,"));

        const auto radians = make_field("{grid: [90, 90]}", [](double lat, double) {
            return lat > 0 ? MISSING : lat < 0 ? -M_PI_2 : 3. * M_PI_2;
        });

        EXPECT(contains(execute("angle-radian-asymmetric", radians), "Angle[mean=4.71239,"));
        EXPECT(contains(execute("angle-radian-symmetric", radians), "Angle[mean=-1.5708,"));
    }


    SECTION("statistics are reset on each execution") {
        param::SimpleParametrisation param;
        std::unique_ptr<Statistics> stats(StatisticsFactory::build("scalar", param));

        stats->execute(make_field("{grid: [90, 90]}", [](double, double) { return 1.; }));
        stats->execute(make_field("{grid: [90, 90]}", [](double lat, double) { return lat; }));

        auto str = to_string(*stats);
        EXPECT(contains(str, "Statistics[Counter[count=12,missing=0,max=90,maxIndex=0,min=-90,minIndex=8],Scalar["));
        EXPECT(contains(str, ",stddev=76.7523,"));
        EXPECT(contains(str, ",kurtosis=-1.5]]"));
    }
}


CASE("Integral") {
    param::SimpleParametrisation param;

    for (const auto& grid : std::vector<std::string>{"{grid: [10, 10]}", "{grid: O8}", "{grid: F8}"}) {
        Log::info() << "Test " << grid << std::endl;

        statistics::Integral integral(param);
        EXPECT(std::isnan(integral.integral()));

        Statistics& stats = integral;

        // weighted average, by latitude (cosine) and number of points per latitude
        stats.execute(make_field(grid, [](double, double) { return 3.; }));
        EXPECT(eckit::types::is_approximately_equal(integral.integral(), 3., 1e-12));
        EXPECT(contains(to_string(stats), "Integral[3"));

        // symmetric about the equator
        stats.execute(make_field(grid, [](double lat, double) { return std::sin(lat * M_PI / 180.); }));
        EXPECT(std::abs(integral.integral()) < 1e-12);

        integral.reset();
        EXPECT(std::isnan(integral.integral()));
    }

    statistics::Integral integral(param);
    Statistics& stats = integral;
    EXPECT_THROWS_AS(stats.execute(make_field("{grid: [10, 10]}", [](double, double) { return MISSING; })),
                     eckit::AssertionFailed);
}


CASE("GridBoxIntegral") {
    param::SimpleParametrisation param;
    statistics::GridBoxIntegral integral(param);
    EXPECT(std::isnan(integral.integral()));

    Statistics& stats = integral;
    const auto R      = util::Earth::radius();

    // globe area
    stats.execute(make_field("{grid: [10, 10]}", [](double, double) { return 1.; }));
    EXPECT(eckit::types::is_approximately_equal(integral.integral() / (4. * M_PI * R * R), 1., 1e-9));
    EXPECT(contains(to_string(stats), "GridBoxIntegral["));

    // area south of the grid boxes edge at 5N
    stats.execute(make_field("{grid: [10, 10]}", [](double lat, double) { return lat > 0. ? 0. : 2.; }));
    auto expected = 2. * 2. * M_PI * R * R * (1. + std::sin(5. * M_PI / 180.));
    EXPECT(eckit::types::is_approximately_equal(integral.integral() / expected, 1., 1e-9));

    integral.reset();
    EXPECT(std::isnan(integral.integral()));
}


CASE("SimplePackingEntropy") {
    struct test_t {
        std::string key;
        long value;
        size_t bucketCount;
    };

    for (const auto& test : std::vector<test_t>{{"", 0, 65536}, {"entropy-buckets", 16, 16}, {"entropy-bits", 4, 16}}) {
        param::SimpleParametrisation param;
        if (!test.key.empty()) {
            param.set(test.key, test.value);
        }

        statistics::SimplePackingEntropy entropy(param);

        EXPECT_EQUAL(entropy.bucketCount(), test.bucketCount);
        EXPECT(std::isnan(entropy.entropy()));
        EXPECT(std::isnan(entropy.scale()));
        EXPECT(contains(to_string(static_cast<const Statistics&>(entropy)),
                        ",entropy=nan,scale=nan,bucketCount=" + std::to_string(test.bucketCount) + "]"));
    }
}


CASE("Spectral") {
    // T2 coefficients (complex, m-major), the last zonal (m = 0, n = 2) one being zero
    const MIRValuesVector values{2., 0., 3., 0., 0., 0., 1., 1., 0.5, 0., 2., 0.};

    param::SimpleParametrisation param;
    statistics::Spectral spectral(param);
    EXPECT(std::isnan(spectral.mean()));

    spectral.execute(make_field("{grid: T2}", values));

    // variance: zonal (m = 0, n > 0) real parts squared, and others twice the modulus squared
    EXPECT_EQUAL(spectral.mean(), 2.);
    EXPECT_EQUAL(spectral.variance(), 9. + 4. + 0.5 + 8.);
    EXPECT_EQUAL(spectral.standardDeviation(), std::sqrt(21.5));
    EXPECT_EQUAL(spectral.enorm(), std::sqrt(4. + 21.5));
    EXPECT_EQUAL(to_string(static_cast<const Statistics&>(spectral)),
                 "Spectral[mean=2,variance=21.5,stddev=4.63681,enorm=5.04975]");

    EXPECT_EQUAL(execute("spectral", make_field("{grid: T2}", values)),
                 "Spectral[mean=2,variance=21.5,stddev=4.63681,enorm=5.04975]");

    spectral.reset();
    EXPECT(std::isnan(spectral.enorm()));

    // values size is checked against the truncation
    EXPECT_THROWS(spectral.execute(make_field("{grid: T3}", values)));
}


CASE("StatisticsFactory") {
    std::ostringstream list;
    StatisticsFactory::list(list);

    for (const auto& name :
         std::vector<std::string>{"angle-degree-asymmetric", "angle-degree-symmetric", "angle-radian-asymmetric",
                                  "angle-radian-symmetric", "central-moments", "grid-box-integral", "integral",
                                  "min-max", "p-norms", "scalar", "simple-packing-entropy", "spectral"}) {
        EXPECT(contains(list.str(), name));
    }

    param::SimpleParametrisation param;
    EXPECT_THROWS_AS(StatisticsFactory::build("unknown", param), eckit::SeriousBug);
}


}  // namespace mir::stats::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
