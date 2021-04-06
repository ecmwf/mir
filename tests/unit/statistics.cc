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


#include <memory>
#include <ostream>
#include <vector>

#include "eckit/testing/Test.h"
#include "eckit/types/FloatCompare.h"

#include "mir/param/SimpleParametrisation.h"
#include "mir/stats/field/CentralMomentStats.h"
#include "mir/stats/field/CounterStats.h"
#include "mir/stats/field/ModeStats.h"
#include "mir/util/Log.h"


#define EXPECTV(a)                                      \
    Log::info() << "EXPECT(" << #a << ")" << std::endl; \
    EXPECT(a)

#define EXPECT_APPROX_V(a, b)                                                \
    Log::info() << "EXPECT_APPROX(" << #a << ", " << #b << ")" << std::endl; \
    EXPECT(eckit::types::is_approximately_equal<double>(a, b, 1e-6))


namespace mir {
namespace tests {
namespace unit {


struct case_t {
    size_t count;
    size_t above;
    size_t below;
    double mode;
    double mean;
    double limit;
    double min;
    double max;
    bool disambiguateMax;
    std::vector<double> data;
    friend std::ostream& operator<<(std::ostream& s, const case_t& c) {
        s << "case: { statistic-disambiguate-max: " << c.disambiguateMax << ", data: [";
        auto sep = "";
        for (auto d : c.data) {
            s << sep << d;
            sep = ", ";
        }
        s << "]}";
        return s;
    }
};


CASE("mir::stats::Field") {

    std::vector<case_t> cases{case_t{5, 0, 5, 4., 2.4, 4.5, 1., 4., true, {1, 1, 2, 4, 4}},
                              case_t{5, 0, 5, 1., 2.4, 4.5, 1., 4., false, {1, 1, 2, 4, 4}},
                              case_t{11, 9, 2, 6., 8., 4.5, 1., 22., true, {22, 1, 3, 6, 6, 6, 6, 7, 7, 12, 12}},
                              case_t{7, 2, 5, 2., 4., 4.5, 1., 9., true, {1, 2, 2, 3, 4, 7, 9}}};


    SECTION("ModeIntegral") {
        for (auto& c : cases) {
            param::SimpleParametrisation param;
            param.set("mode-disambiguate-max", c.disambiguateMax);

            std::unique_ptr<stats::Field> statistic(stats::FieldFactory::build("mode-integral", param));
            Log::info() << "Test " << c << ", statistic=" << *statistic << std::endl;

            for (auto d : c.data) {
                statistic->count(d);
            }

            Log::info() << "mode=" << statistic->value() << std::endl;
            EXPECT_APPROX_V(statistic->value(), c.mode);
        }
    }


    SECTION("Mean") {
        for (auto& c : cases) {
            param::SimpleParametrisation param;
            std::unique_ptr<stats::Field> statistic(stats::FieldFactory::build("mean", param));
            Log::info() << "Test " << c << ", statistic=" << *statistic << std::endl;

            for (auto d : c.data) {
                statistic->count(d);
            }

            Log::info() << "mean=" << statistic->value() << std::endl;
            EXPECT_APPROX_V(statistic->value(), c.mean);
        }
    }


    SECTION("Counter") {
        for (auto& c : cases) {
            param::SimpleParametrisation param;
            param.set("counter-lower-limit", c.limit);
            param.set("counter-upper-limit", c.limit);

            param.set("mode-disambiguate-max", c.disambiguateMax);
            param.set("mode-real-values", std::vector<double>{4., 5.});
            param.set("mode-real-min", std::vector<double>{c.limit});

            std::unique_ptr<stats::Field> above(stats::FieldFactory::build("count-above-upper-limit", param));
            std::unique_ptr<stats::Field> below(stats::FieldFactory::build("count-below-lower-limit", param));
            std::unique_ptr<stats::Field> mode(stats::FieldFactory::build("mode-real", param));

            Log::info() << "Test " << c << ", above=" << *above << ", below=" << *below << ", mode=" << *mode
                        << std::endl;

            for (auto d : c.data) {
                above->count(d);
                below->count(d);
                mode->count(d);
            }

            Log::info() << "above=" << above->value() << std::endl;
            Log::info() << "below=" << below->value() << std::endl;
            EXPECT_APPROX_V(above->value(), c.above);
            EXPECT_APPROX_V(below->value(), c.below);
            EXPECT_APPROX_V(above->value() + below->value(), c.count);

            Log::info() << "mode=" << mode->value() << std::endl;
            EXPECT_APPROX_V(mode->value(), above->value() < below->value()? 4. : 5.);
        }
    }
}


}  // namespace unit
}  // namespace tests
}  // namespace mir


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
