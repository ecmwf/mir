// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <cmath>
#include <memory>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

#include "eckit/exception/Exceptions.h"
#include "eckit/log/JSON.h"
#include "eckit/testing/Test.h"
#include "eckit/types/FloatCompare.h"

#include "mir/param/SimpleParametrisation.h"
#include "mir/stats/Field.h"
#include "mir/util/Log.h"

#include "stats.h"


namespace mir::stats::tests {


constexpr double EPS = 1e-6;


bool approx(double a, double b) {
    return eckit::types::is_approximately_equal(a, b, EPS);
}


std::unique_ptr<Field> build(const std::string& name, const param::MIRParametrisation& param,
                             const std::vector<double>& data, bool hasMissing = false) {
    std::unique_ptr<Field> stats(FieldFactory::build(name, param));
    stats->reset(MISSING, hasMissing);

    for (const auto& d : data) {
        stats->count(d);
    }

    return stats;
}


std::string json(const Field& stats) {
    std::ostringstream str;
    eckit::JSON j(str);
    j << stats;
    return str.str();
}


struct case_t {
    double mode;
    double median;
    double mean;
    double min;
    double max;
    bool disambiguateMax;
    std::vector<double> data;
    friend std::ostream& operator<<(std::ostream& s, const case_t& c) {
        s << "case: { statistic-disambiguate-max: " << c.disambiguateMax << ", data: [";
        const auto* sep = "";
        for (const auto& d : c.data) {
            s << sep << d;
            sep = ", ";
        }
        s << "]}";
        return s;
    }
};


CASE("mode, median, mean, count above/below limits") {
    std::vector<case_t> cases{case_t{42., 42., 42, 42., 42., true, {42}},
                              case_t{4., 2., 2.4, 1., 4., true, {1, 1, 2, 4, 4}},
                              case_t{1., 2., 2.4, 1., 4., false, {1, 1, 2, 4, 4}},
                              case_t{6., 6., 8., 1., 22., true, {22, 1, 3, 6, 6, 6, 6, 7, 7, 12, 12}},
                              case_t{2., 3., 4., 1., 9., true, {1, 2, 2, 3, 4, 7, 9}},
                              case_t{3., 6., 5.285714, 1., 9., true, {1, 3, 3, 6, 7, 8, 9}}};


    SECTION("mode-integral") {
        for (const auto& c : cases) {
            param::SimpleParametrisation param;
            param.set("mode-disambiguate-max", c.disambiguateMax);

            Log::info() << "Test " << c << ':' << std::endl;
            EXPECT(approx(build("mode-integral", param, c.data)->value(), c.mode));
        }
    }


    SECTION("median-integral") {
        for (const auto& c : cases) {
            param::SimpleParametrisation param;
            param.set("mode-disambiguate-max", c.disambiguateMax);

            Log::info() << "Test " << c << ':' << std::endl;
            EXPECT(approx(build("median-integral", param, c.data)->value(), c.median));
        }
    }


    SECTION("mean, minimum, maximum") {
        for (const auto& c : cases) {
            param::SimpleParametrisation param;

            Log::info() << "Test " << c << ':' << std::endl;
            EXPECT(approx(build("mean", param, c.data)->value(), c.mean));
            EXPECT_EQUAL(build("minimum", param, c.data)->value(), c.min);
            EXPECT_EQUAL(build("maximum", param, c.data)->value(), c.max);
        }
    }


    SECTION("count-above-upper-limit, count-below-lower-limit, mode-real") {
        for (const auto& c : cases) {
            const std::vector<double> modeValues{4., 5.};
            const std::vector<double> modeLimits{4.5};

            param::SimpleParametrisation param;
            param.set("counter-lower-limit", modeLimits.back());
            param.set("counter-upper-limit", modeLimits.back());

            param.set("mode-disambiguate-max", c.disambiguateMax);
            param.set("mode-real-values", modeValues);
            param.set("mode-real-min", modeLimits);

            Log::info() << "Test " << c << ':' << std::endl;

            auto above = build("count-above-upper-limit", param, c.data);
            auto below = build("count-below-lower-limit", param, c.data);
            auto mode  = build("mode-real", param, c.data);

            EXPECT(approx(above->value() + below->value(), static_cast<double>(c.data.size())));
            EXPECT(approx(mode->value(), above->value() < below->value() ? modeValues.front() : modeValues.back()));
        }
    }
}


CASE("all statistics, on values with missing values") {
    // mean 5, sum of squared deviations 32, of cubed deviations 42, of 4th powers 356
    const std::vector<double> data{MISSING, 2., 4., 4., 4., MISSING, 5., 5., 7., 9.};

    param::SimpleParametrisation param;
    param.set("counter-lower-limit", 4.5);
    param.set("counter-upper-limit", 6.);
    param.set("mode-real-values", std::vector<double>{4., 5.});
    param.set("mode-real-min", std::vector<double>{4.5});
    param.set("mode-disambiguate-max", true);
    param.set("mode-boolean-min", 4.5);

    struct test_t {
        std::string name;
        double value;
        std::string print;
    };

    for (const auto& test : std::vector<test_t>{
             {"count", 8., "Count[8]"},
             {"count-above-upper-limit", 2., "CountAboveUpperLimit[2]"},
             {"count-below-lower-limit", 4., "CountBelowLowerLimit[4]"},
             {"maximum", 9., "Maximum[9]"},
             {"minimum", 2., "Minimum[2]"},
             {"mean", 5., "Mean[]"},
             {"variance", 32. / 7., "Variance[]"},
             {"stddev", std::sqrt(32. / 7.), "StandardDeviation[]"},
             {"skewness", 42. / 64., "Skewness[]"},
             {"kurtosis", 356. * 8. / (32. * 32.) - 3., "Kurtosis[]"},
             {"mode-integral", 4., "ModeIntegral[4]"},
             {"mode-real", 5., "ModeReal[5]"},
             {"mode-boolean", 1., "ModeBoolean[1]"},
             {"median-boolean", 1., "ModeBoolean[1]"},
             {"median-integral", 5., "MedianIntegral[5]"},
         }) {
        Log::info() << "Test '" << test.name << "'" << std::endl;

        auto stats = build(test.name, param, data, true);
        EXPECT(approx(stats->value(), test.value));
        EXPECT_EQUAL(to_string(*stats), test.print);

        auto type = test.name == "median-boolean" ? std::string("mode-boolean") : test.name;
        EXPECT(contains(json(*stats), "{\"type\":\"" + type + "\",\"value\":"));
    }

    // without missing values, MISSING is a value as any other
    EXPECT_EQUAL(build("maximum", param, data, false)->value(), MISSING);
    EXPECT_EQUAL(build("count", param, data, false)->value(), 10.);
}


CASE("sum") {
    param::SimpleParametrisation param;
    EXPECT(approx(build("sum", param, {2., 4., 4., 4., 5., 5., 7., 9.})->value(), 40.));
    EXPECT(approx(build("sum", param, {-1.5})->value(), -1.5));
    EXPECT(approx(build("sum", param, {1., MISSING, 2.}, true)->value(), 3.));
    EXPECT_EQUAL(json(*build("sum", param, {1., 2.})), "{\"type\":\"sum\",\"value\":3}");
    EXPECT_EQUAL(to_string(*build("sum", param, {1., 2.})), "Sum[]");
}


CASE("empty and reset") {
    param::SimpleParametrisation param;
    param.set("mode-boolean-min", 0.5);
    param.set("mode-disambiguate-max", true);

    // undefined values are not output to JSON
    for (const auto& name :
         std::vector<std::string>{"mean", "minimum", "maximum", "mode-integral", "mode-boolean", "median-integral"}) {
        auto stats = build(name, param, {});
        EXPECT(std::isnan(stats->value()));
        EXPECT_EQUAL(json(*stats), "{\"type\":\"" + name + "\"}");
    }

    for (const auto& name : std::vector<std::string>{"count", "variance", "stddev", "skewness", "kurtosis"}) {
        EXPECT_EQUAL(build(name, param, {})->value(), 0.);
    }

    // all values missing
    EXPECT(std::isnan(build("mean", param, {MISSING, MISSING}, true)->value()));
    EXPECT_EQUAL(build("count", param, {MISSING, MISSING}, true)->value(), 0.);

    auto stats = build("mean", param, {1., 2., 3.});
    EXPECT_EQUAL(stats->value(), 2.);

    stats->reset(3., true);
    for (const auto& v : std::vector<double>{3., 10.}) {
        stats->count(v);
    }
    EXPECT_EQUAL(stats->value(), 10.);
}


CASE("FieldFactory") {
    std::ostringstream list;
    FieldFactory::list(list);

    for (const auto& name :
         std::vector<std::string>{"count", "count-above-upper-limit", "count-below-lower-limit", "kurtosis", "maximum",
                                  "mean", "median-boolean", "median-integral", "minimum", "mode-boolean",
                                  "mode-integral", "mode-real", "skewness", "stddev", "sum", "variance"}) {
        EXPECT(contains(list.str(), name));
    }

    param::SimpleParametrisation param;
    EXPECT_THROWS_AS(FieldFactory::build("unknown", param), eckit::SeriousBug);
}


}  // namespace mir::stats::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
