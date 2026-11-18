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
#include "mir/stats/Method.h"

#include "stats.h"


namespace mir::stats::tests {


bool approx(double a, double b) {
    return eckit::types::is_approximately_equal(a, b, 1e-9);
}


std::unique_ptr<Method> build(const std::string& name, size_t size) {
    static const param::SimpleParametrisation param;
    std::unique_ptr<Method> method(MethodFactory::build(name, param));
    method->resize(size);
    return method;
}


/// Field without representation (statistics by point only use the values)
data::MIRField make_field(const MIRValuesVector& values) {
    return make_field(static_cast<const repres::Representation*>(nullptr), values);
}


CASE("MethodT (statistics by point, over fields)") {
    const std::string GRID = "{grid: [90, 90]}";


    SECTION("mean, with missing values") {
        // point 0 is always missing, point 1 sometimes, point i > 1 has values i, 2 i and 3 i
        auto method = build("scalar", 12);
        for (size_t k = 1; k <= 3; ++k) {
            MIRValuesVector values(12);
            for (size_t i = 0; i < values.size(); ++i) {
                values[i] = i == 0 || (i == 1 && k == 2) ? MISSING : static_cast<double>(i * k);
            }
            method->execute(make_field(GRID, values));
        }

        auto out = make_field(GRID, MIRValuesVector(12, 0.));
        EXPECT_NOT(out.hasMissing());

        method->mean(out);
        EXPECT(out.hasMissing());

        const auto& mean = out.values(0);
        EXPECT_EQUAL(mean[0], MISSING);
        EXPECT(approx(mean[1], 2.));
        for (size_t i = 2; i < mean.size(); ++i) {
            EXPECT(approx(mean[i], 2. * static_cast<double>(i)));
        }

        // the counter (of the last field)
        EXPECT(contains(to_string(*method), "Method[Counter[count=12,missing=1,max=33,maxIndex=11,"));
    }


    SECTION("variance and standard deviation (single point)") {
        for (const auto& name : std::vector<std::string>{"scalar", "central-moments"}) {
            auto method = build(name, 1);
            for (const auto& v : std::vector<double>{2., 4., 4., 4., 5., 5., 7., 9.}) {
                method->execute(make_field({v}));
            }

            auto out = make_field({0.});

            method->mean(out);
            EXPECT(approx(out.values(0)[0], 5.));

            method->variance(out);
            EXPECT(approx(out.values(0)[0], 32. / 7.));

            method->stddev(out);
            EXPECT(approx(out.values(0)[0], std::sqrt(32. / 7.)));
        }
    }


    SECTION("variance and standard deviation (by point)") {
        // point i has values i, 2 i and 3 i (variance i^2)
        auto method = build("central-moments", 12);
        for (size_t k = 1; k <= 3; ++k) {
            MIRValuesVector values(12);
            for (size_t i = 0; i < values.size(); ++i) {
                values[i] = static_cast<double>(i * k);
            }
            method->execute(make_field(GRID, values));
        }

        auto out = make_field(GRID, MIRValuesVector(12, 0.));

        method->variance(out);
        for (size_t i = 0; i < 12; ++i) {
            EXPECT(approx(out.values(0)[i], static_cast<double>(i * i)));
        }

        method->stddev(out);
        for (size_t i = 0; i < 12; ++i) {
            EXPECT(approx(out.values(0)[i], static_cast<double>(i)));
        }
    }


    SECTION("angles") {
        struct test_t {
            std::string name;
            double a;
            double b;
            double mean;
        };

        for (const auto& test : std::vector<test_t>{{"angle.degree.asymmetric", -90., 270., 270.},
                                                    {"angle.degree.symmetric", -90., 270., -90.},
                                                    {"angle.radian.asymmetric", -M_PI_2, 3. * M_PI_2, 3. * M_PI_2},
                                                    {"angle.radian.symmetric", -M_PI_2, 3. * M_PI_2, -M_PI_2}}) {
            auto method = build(test.name, 1);
            method->execute(make_field({test.a}));
            method->execute(make_field({test.b}));

            auto out = make_field({0.});
            method->mean(out);
            EXPECT(eckit::types::is_approximately_equal(out.values(0)[0], test.mean, 1e-9));
        }
    }


    SECTION("fields must match the size") {
        auto method = build("scalar", 2);
        EXPECT_THROWS_AS(method->execute(make_field({1.})), eckit::AssertionFailed);

        auto out = make_field({0.});
        EXPECT_THROWS_AS(method->mean(out), eckit::AssertionFailed);
        EXPECT_THROWS_AS(method->variance(out), eckit::AssertionFailed);
        EXPECT_THROWS_AS(method->stddev(out), eckit::AssertionFailed);

        auto empty = build("scalar", 0);
        EXPECT_THROWS_AS(empty->execute(make_field({1.})), eckit::AssertionFailed);
    }
}


CASE("MethodFactory") {
    std::ostringstream list;
    MethodFactory::list(list);

    for (const auto& name :
         std::vector<std::string>{"angle.degree.asymmetric", "angle.degree.symmetric", "angle.radian.asymmetric",
                                  "angle.radian.symmetric", "central-moments", "scalar"}) {
        EXPECT(contains(list.str(), name));
    }

    param::SimpleParametrisation param;
    EXPECT_THROWS_AS(MethodFactory::build("unknown", param), eckit::SeriousBug);
}


}  // namespace mir::stats::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
