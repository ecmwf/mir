// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <cmath>
#include <limits>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "eckit/exception/Exceptions.h"
#include "eckit/testing/Test.h"

#include "mir/stats/Distribution.h"
#include "mir/util/Log.h"

#include "stats.h"


namespace mir::stats::tests {


std::unique_ptr<Distribution> build(const std::string& spec) {
    return std::unique_ptr<Distribution>(DistributionFactory::build(spec));
}


CASE("distributions (values range)") {
    constexpr double inf = std::numeric_limits<double>::infinity();

    struct test_t {
        std::string spec;
        double min;
        double max;
        std::string print;
    };

    for (const auto& test : std::vector<test_t>{
             // degenerate (constant)
             {"bernoulli-distribution{p:1}", 1., 1., "bernoulli-distribution{p:1.000000}"},
             {"bernoulli-distribution{p:0}", 0., 0., "bernoulli-distribution{p:0.000000}"},
             {"binomial-distribution{t:5,p:1}", 5., 5., "binomial-distribution{t:5,p:1.000000}"},
             {"discrete-distribution{probabilities:[0,1,0]}", 1., 1.,
              "discrete-distribution{probabilities:[0.000000,1.000000,0.000000]}"},
             {"geometric-distribution{p:1}", 0., 0., "geometric-distribution{p:1.000000}"},
             {"negative-binomial-distribution{k:1,p:1}", 0., 0., "negative-binomial-distribution{k:1,"},
             {"uniform-int-distribution{a:3,b:3}", 3., 3., "uniform-int-distribution{a:3,b:3},min=3,max=3"},

             // bounded
             {"bernoulli-distribution{}", 0., 1., "bernoulli-distribution{p:0.500000}"},
             {"binomial-distribution{t:4}", 0., 4., "binomial-distribution{t:4,p:0.500000}"},
             {"uniform-int-distribution{a:-2,b:2}", -2., 2., "uniform-int-distribution{a:-2,b:2}"},
             {"uniform-int-distribution{}", 0., 1., "uniform-int-distribution{a:0,b:1}"},
             {"uniform-real-distribution{a:1,b:2}", 1., 2., "uniform-real-distribution{a:1.000000,b:2.000000}"},
             {"uniform-real-distribution{}", 0., 1., "uniform-real-distribution{a:0.000000,b:1.000000}"},
             {"piecewise-constant-distribution{intervals:[0,1,2],densities:[0,1]}", 1., 2.,
              "piecewise-constant-distribution{densities:[0.000000,1.000000],intervals:[0.000000,1.000000,2.000000]}"},
             // (densities are normalised)
             {"piecewise-linear-distribution{intervals:[0,1,2],densities:[0,0,1]}", 1., 2.,
              "piecewise-linear-distribution{densities:[0.000000,0.000000,2.000000]"},

             // positive
             {"chi-squared-distribution{n:2}", 0., inf, "chi-squared-distribution{n:2.000000}"},
             {"exponential-distribution{lambda:2}", 0., inf, "exponential-distribution{lambda:2.000000}"},
             {"fisher-f-distribution{m:2,n:3}", 0., inf, "fisher-f-distribution{m:2.000000,"},
             {"gamma-distribution{alpha:2,beta:3}", 0., inf, "gamma-distribution{alpha:2.000000,"},
             {"geometric-distribution{p:0.3}", 0., inf, "geometric-distribution{p:0.300000}"},
             {"lognormal-distribution{m:1,s:2}", 0., inf, "lognormal-distribution{m:1.000000,"},
             {"negative-binomial-distribution{k:3,p:0.4}", 0., inf, "negative-binomial-distribution{k:3,"},
             {"poisson-distribution{mean:4}", 0., inf, "poisson-distribution{mean:4.000000}"},
             {"weibull-distribution{a:2,b:3}", 0., inf, "weibull-distribution{a:2.000000,b:3.000000}"},

             // unbounded
             {"cauchy-distribution{a:1,b:2}", -inf, inf, "cauchy-distribution{a:1.000000,b:2.000000}"},
             {"extreme-value-distribution{}", -inf, inf, "extreme-value-distribution{a:0.000000,b:1.000000}"},
             {"normal-distribution{mean:1,stddev:2}", -inf, inf, "normal-distribution{mean:1.000000,"},
             {"student-t-distribution{n:3}", -inf, inf, "student-t-distribution{n:3.000000}"},
         }) {
        Log::info() << "Test '" << test.spec << "'" << std::endl;

        auto d = build(test.spec);
        EXPECT(contains(to_string(*d), "Distribution[" + test.print));

        for (size_t i = 0; i < 100; ++i) {
            auto value = (*d)();
            EXPECT(!std::isnan(value));
            EXPECT(test.min <= value && value <= test.max);
        }

        d->reset();
        EXPECT(!std::isnan((*d)()));
    }
}


CASE("distributions (arguments)") {
    SECTION("default arguments") {
        EXPECT(contains(to_string(*build("normal-distribution{}")), "{mean:0.000000,stddev:1.000000}"));
        EXPECT(contains(to_string(*build("gamma-distribution{}")), "{alpha:1.000000,beta:1.000000}"));
        EXPECT(contains(to_string(*build("fisher-f-distribution{}")), "{m:1.000000,n:1.000000}"));
        EXPECT(contains(to_string(*build("lognormal-distribution{}")), "{m:0.000000,s:1.000000}"));
        EXPECT(contains(to_string(*build("negative-binomial-distribution{}")), "{k:1,p:0.500000}"));

        EXPECT(contains(to_string(*build("chi-squared-distribution{}")), "{n:1.000000}"));
        EXPECT(contains(to_string(*build("exponential-distribution{}")), "{lambda:1.000000}"));
        EXPECT(contains(to_string(*build("poisson-distribution{}")), "{mean:1.000000}"));
        EXPECT(contains(to_string(*build("student-t-distribution{}")), "{n:1.000000}"));
        EXPECT(contains(to_string(*build("weibull-distribution{}")), "{a:1.000000,b:1.000000}"));
        EXPECT(contains(to_string(*build("cauchy-distribution{}")), "{a:0.000000,b:1.000000}"));
        EXPECT(contains(to_string(*build("geometric-distribution{}")), "{p:0.500000}"));
        EXPECT(contains(to_string(*build("binomial-distribution{}")), "{t:1,p:0.500000}"));
    }


    SECTION("sample mean (normal-distribution)") {
        auto d = build("normal-distribution{mean:42,stddev:3}");

        // standard error 0.03, a tolerance of 1 is safe
        constexpr size_t N = 10000;
        double sum         = 0.;
        for (size_t i = 0; i < N; ++i) {
            sum += (*d)();
        }

        EXPECT(std::abs(sum / N - 42.) < 1.);
    }


    SECTION("intervals and densities") {
        auto c = build("piecewise-constant-distribution{intervals:[0,1,3],densities:[1,1]}");
        EXPECT(contains(to_string(*c), "intervals:[0.000000,1.000000,3.000000]}"));

        auto l = build("piecewise-linear-distribution{intervals:[0,1],densities:[1,1]}");
        EXPECT(contains(to_string(*l), "intervals:[0.000000,1.000000]}"));
    }


    SECTION("invalid arguments") {
        EXPECT_THROWS_AS(build("discrete-distribution{}"), eckit::AssertionFailed);
        EXPECT_THROWS_AS(build("piecewise-constant-distribution{intervals:[0,1,2],densities:[1]}"),
                         eckit::AssertionFailed);
        EXPECT_THROWS_AS(build("piecewise-constant-distribution{intervals:[0,1]}"), eckit::AssertionFailed);
        EXPECT_THROWS_AS(build("piecewise-linear-distribution{intervals:[0,1,2],densities:[1,1]}"),
                         eckit::AssertionFailed);
        EXPECT_THROWS_AS(build("piecewise-linear-distribution{intervals:[0,1]}"), eckit::AssertionFailed);
    }
}


CASE("DistributionFactory") {
    std::ostringstream list;
    DistributionFactory::list(list);

    for (const auto& name : std::vector<std::string>{"bernoulli-distribution",
                                                     "binomial-distribution",
                                                     "cauchy-distribution",
                                                     "chi-squared-distribution",
                                                     "discrete-distribution",
                                                     "exponential-distribution",
                                                     "extreme-value-distribution",
                                                     "fisher-f-distribution",
                                                     "gamma-distribution",
                                                     "geometric-distribution",
                                                     "lognormal-distribution",
                                                     "negative-binomial-distribution",
                                                     "normal-distribution",
                                                     "piecewise-constant-distribution",
                                                     "piecewise-linear-distribution",
                                                     "poisson-distribution",
                                                     "student-t-distribution",
                                                     "uniform-int-distribution",
                                                     "uniform-real-distribution",
                                                     "weibull-distribution"}) {
        EXPECT(contains(list.str(), name));
    }

    EXPECT_THROWS_AS(build("unknown-distribution{}"), eckit::SeriousBug);
    EXPECT_THROWS_AS(build("unknown-distribution"), eckit::SeriousBug);

    // arguments are optional
    std::unique_ptr<Distribution> normal(build("normal-distribution"));
    EXPECT(normal);
    EXPECT(contains(to_string(*normal), "normal-distribution{"));

    // arguments are in braces
    EXPECT_THROWS_AS(build("normal-distribution{mean:1"), eckit::AssertionFailed);
}


}  // namespace mir::stats::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
