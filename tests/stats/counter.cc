// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <cmath>
#include <sstream>
#include <string>
#include <vector>

#include "eckit/exception/Exceptions.h"
#include "eckit/testing/Test.h"

#include "mir/param/CombinedParametrisation.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/stats/detail/Counter.h"
#include "mir/stats/detail/CounterBinary.h"

#include "stats.h"


namespace mir::stats::tests {


using detail::Counter;
using detail::CounterBinary;


template <typename T>
std::string printed(const T& counter) {
    std::ostringstream str;
    counter.print(str);
    return str.str();
}


CASE("Counter") {
    SECTION("count, min/max (and their index), limits and missing values") {
        Counter counter(MISSING, true, 2., 5.);

        for (const auto& v : std::vector<double>{3., MISSING, 1., 6., MISSING, 5., 2.}) {
            counter.count(v);
        }

        // missing values are counted (and indexed) but don't take part in the statistics
        EXPECT_EQUAL(counter.count(), 7);
        EXPECT_EQUAL(counter.missing(), 2);
        EXPECT_EQUAL(counter.countBelowLowerLimit(), 1);
        EXPECT_EQUAL(counter.countAboveUpperLimit(), 1);
        EXPECT_EQUAL(counter.min(), 1.);
        EXPECT_EQUAL(counter.minIndex(), 2);
        EXPECT_EQUAL(counter.max(), 6.);
        EXPECT_EQUAL(counter.maxIndex(), 3);
        EXPECT_EQUAL(counter.missingValue(), MISSING);

        EXPECT_EQUAL(printed(counter),
                     "Counter[count=7,missing=2,countAboveUpperLimit=1,countBelowLowerLimit=1,max=6,maxIndex=3,min=1,"
                     "minIndex=2]");
    }


    SECTION("count returns if the value is not missing") {
        Counter counter(MISSING, true);

        EXPECT(counter.count(1.));
        EXPECT_NOT(counter.count(MISSING));

        Counter nomissing(MISSING, false);
        EXPECT(nomissing.count(MISSING));
        EXPECT_EQUAL(nomissing.missing(), 0);
        EXPECT_EQUAL(nomissing.max(), MISSING);
        EXPECT(std::isnan(nomissing.missingValue()));
    }


    SECTION("first value sets min/max (including the first non-missing value)") {
        Counter counter(MISSING, true);

        counter.count(MISSING);
        counter.count(-3.);

        EXPECT_EQUAL(counter.min(), -3.);
        EXPECT_EQUAL(counter.max(), -3.);
        EXPECT_EQUAL(counter.minIndex(), 1);
        EXPECT_EQUAL(counter.maxIndex(), 1);
    }


    SECTION("empty (min/max are NaN, limits not printed if not set)") {
        Counter counter(MISSING, false);

        EXPECT_EQUAL(counter.count(), 0);
        EXPECT(std::isnan(counter.min()));
        EXPECT(std::isnan(counter.max()));
        EXPECT_EQUAL(printed(counter), "Counter[count=0,missing=0,max=nan,maxIndex=0,min=nan,minIndex=0]");
    }


    SECTION("reset (by values, and by field)") {
        Counter counter(MISSING, false, 0., 0.);
        for (const auto& v : std::vector<double>{-1., 1., MISSING}) {
            counter.count(v);
        }

        EXPECT_EQUAL(counter.count(), 3);
        EXPECT_EQUAL(counter.missing(), 0);

        counter.reset(1., true);

        EXPECT_EQUAL(counter.count(), 0);
        EXPECT_EQUAL(counter.countBelowLowerLimit(), 0);
        EXPECT_EQUAL(counter.countAboveUpperLimit(), 0);
        EXPECT(std::isnan(counter.min()));
        EXPECT(std::isnan(counter.max()));

        counter.count(1.);
        EXPECT_EQUAL(counter.missing(), 1);

        auto field = make_field("{grid: [90, 90]}", [](double lat, double) { return lat > 0 ? MISSING : lat; });
        EXPECT(field.hasMissing());

        counter.reset(field);
        for (const auto& v : field.values(0)) {
            counter.count(v);
        }

        EXPECT_EQUAL(counter.count(), 12);
        EXPECT_EQUAL(counter.missing(), 4);
        EXPECT_EQUAL(counter.min(), -90.);
        EXPECT_EQUAL(counter.max(), 0.);
        EXPECT_EQUAL(counter.countBelowLowerLimit(), 4);
        EXPECT_EQUAL(counter.countAboveUpperLimit(), 0);
    }


    SECTION("limits by parametrisation") {
        param::SimpleParametrisation param;
        param.set("counter-lower-limit", 0.);
        param.set("counter-upper-limit", 10.);

        Counter counter(param);
        for (const auto& v : std::vector<double>{-1., 0., 5., 10., 11., 12.}) {
            counter.count(v);
        }

        EXPECT_EQUAL(counter.countBelowLowerLimit(), 1);
        EXPECT_EQUAL(counter.countAboveUpperLimit(), 2);
        EXPECT(std::isnan(counter.missingValue()));

        Counter nolimits{param::SimpleParametrisation()};
        nolimits.count(-1e9);
        nolimits.count(1e9);
        EXPECT_EQUAL(nolimits.countBelowLowerLimit(), 0);
        EXPECT_EQUAL(nolimits.countAboveUpperLimit(), 0);
    }
}


CASE("CounterBinary") {
    // fields only provide the missing values settings
    auto field1 = make_field("{grid: [90, 90]}", [](double, double) { return MISSING; });
    auto field2 = make_field("{grid: [90, 90]}", [](double, double) { return MISSING; });
    EXPECT(field1.hasMissing() && field2.hasMissing());

    struct value_t {
        double a;
        double b;
    };

    const std::vector<value_t> values{{1., 1.}, {2., 2.5}, {MISSING, 4.}, {3., MISSING}, {MISSING, MISSING}, {6., 7.}};

    auto counted = [&](CounterBinary& counter) {
        counter.reset(field1, field2);
        for (const auto& v : values) {
            counter.count(v.a, v.b, std::abs(v.a - v.b));
        }
    };


    SECTION("missing values and maximum difference") {
        param::SimpleParametrisation param1;
        param::SimpleParametrisation param2;
        CounterBinary counter(param1, param2);

        counter.reset(field1, field2);
        EXPECT(counter.count(1., 1., 0.));
        EXPECT_NOT(counter.count(MISSING, 1., 0.));
        EXPECT_EQUAL(counter.missingIn1NotIn2(), 1);

        counted(counter);

        EXPECT_EQUAL(counter.count(), 6);
        EXPECT_EQUAL(counter.missing(), 1);
        EXPECT_EQUAL(counter.missingIn1NotIn2(), 1);
        EXPECT_EQUAL(counter.missingIn2NotIn1(), 1);
        EXPECT_EQUAL(counter.max(), 1.);
        EXPECT_EQUAL(counter.maxIndex(), 5);
        EXPECT_EQUAL(counter.ignoreDifferentMissingValues(), 0);
        EXPECT_EQUAL(counter.ignoreAboveUpperLimit(), 0);

        // default is an exact comparison
        auto reasons = counter.check();
        EXPECT(contains(reasons, "* different missing values (2) greater than 0"));
        EXPECT(contains(reasons, "* maximum difference (1) greater than 0"));

        auto str = printed(counter);
        EXPECT(contains(str,
                        "CounterBinary[count=6,max=1,maxIndex=5,maxIndexValue1=6,maxIndexValue2=7,missing=1,"
                        "missingIn1NotIn2=1,missingIn2NotIn1=1,toleranceType=Absolute,tolerance=0,counter1="));
    }


    SECTION("tolerances (absolute, ignoring different missing values)") {
        param::SimpleParametrisation param1;
        param1.set("absolute-error", 1.);
        param1.set("ignore-different-missing-values", size_t(2));

        param::SimpleParametrisation param2;
        CounterBinary counter(param1, param2);

        counted(counter);
        EXPECT_EQUAL(counter.check(), "");
    }


    SECTION("tolerances (absolute, ignoring different missing values by a factor of the count)") {
        param::SimpleParametrisation param1;
        param1.set("absolute-error", 0.5);
        param1.set("ignore-different-missing-values-factor", 0.5);

        param::SimpleParametrisation param2;
        param2.set("absolute-error", 0.5);
        CounterBinary counter(param1, param2);

        counted(counter);
        EXPECT_EQUAL(counter.ignoreDifferentMissingValues(), 3);

        auto reasons = counter.check();
        EXPECT_NOT(contains(reasons, "missing"));
        EXPECT(contains(reasons, "* maximum difference (1) greater than 0.5"));
    }


    SECTION("different tolerances are not used, the comparison is exact") {
        param::SimpleParametrisation param1;
        param1.set("absolute-error", 2.);

        param::SimpleParametrisation param2;
        param2.set("absolute-error", 3.);

        CounterBinary counter(param1, param2);

        counted(counter);
        EXPECT(contains(counter.check(), "* maximum difference (1) greater than 0"));
    }


    SECTION("limits (of the difference)") {
        param::SimpleParametrisation param1;
        param1.set("absolute-error", 1.);
        param1.set("ignore-different-missing-values", size_t(2));
        param1.set("counter-lower-limit", 0.25);
        param1.set("counter-upper-limit", 0.75);

        param::SimpleParametrisation param2;
        CounterBinary counter(param1, param2);

        counted(counter);
        EXPECT_EQUAL(counter.countBelowLowerLimit(), 1);
        EXPECT_EQUAL(counter.countAboveUpperLimit(), 1);
        EXPECT_EQUAL(counter.check(), "\n* counter above limit 0.75 (1) greater than 0");

        auto str = printed(counter);
        EXPECT(contains(str, ",countAboveUpperLimit=1,countBelowLowerLimit=1,"));

        param1.set("ignore-above-upper-limit", size_t(1));
        CounterBinary ignore1(param1, param2);
        counted(ignore1);
        EXPECT_EQUAL(ignore1.check(), "");

        param1.set("ignore-above-upper-limit-factor", 0.1);
        CounterBinary ignore2(param1, param2);
        counted(ignore2);
        EXPECT_EQUAL(ignore2.ignoreAboveUpperLimit(), 0);
        EXPECT_EQUAL(ignore2.check(), "\n* counter above limit 0.75 (1) greater than 0");
    }


    SECTION("relative errors (at the maximum difference, of the minima, of the maxima)") {
        param::SimpleParametrisation param1;
        param1.set("ignore-different-missing-values", size_t(2));
        param1.set("relative-error-factor", 0.1);
        param1.set("relative-error-min", 0.1);
        param1.set("relative-error-max", 0.1);

        param::SimpleParametrisation param2;
        CounterBinary counter(param1, param2);

        counted(counter);

        // |6 - 7| / 7 at the maximum difference, and of the maxima; minima are both 1
        auto reasons = counter.check();
        EXPECT(contains(reasons, "* relative error (0.142857) greater than 0.1"));
        EXPECT(contains(reasons, "* maximum relative error (0.142857) greater than 0.1"));
        EXPECT_NOT(contains(reasons, "minimum relative error"));
        EXPECT_NOT(contains(reasons, "maximum difference"));

        auto str = printed(counter);
        EXPECT(contains(str, "toleranceType=RelativeError,toleranceFactor=0.1"));
        EXPECT(contains(str, "toleranceType=RelativeMin,tolerance=0.1"));
        EXPECT(contains(str, "toleranceType=RelativeMax,tolerance=0.1"));
        EXPECT_NOT(contains(str, "toleranceType=Absolute"));

        param1.set("relative-error-factor", 0.15);
        param1.set("relative-error-max", 0.15);
        CounterBinary loose(param1, param2);

        counted(loose);
        EXPECT_EQUAL(loose.check(), "");
    }


    SECTION("relative errors of zeros") {
        param::SimpleParametrisation param1;
        param1.set("relative-error-min", 0.);

        param::SimpleParametrisation param2;
        CounterBinary counter(param1, param2);

        counter.reset(field1, field2);
        counter.count(0., 0., 0.);
        counter.count(1., 1., 0.);

        EXPECT_EQUAL(counter.check(), "");
    }


    SECTION("packing error (as a factor of the fields packing error)") {
        param::SimpleParametrisation metadata;
        metadata.set("packingError", 0.5);

        param::SimpleParametrisation user1;
        user1.set("ignore-different-missing-values", size_t(2));
        user1.set("packing-error-factor", 2.);

        param::SimpleParametrisation user2;

        param::CombinedParametrisation param1(user1, metadata);
        param::CombinedParametrisation param2(user2, metadata);

        CounterBinary counter(param1, param2);
        counted(counter);
        EXPECT_EQUAL(counter.check(), "");
        EXPECT(contains(printed(counter), "toleranceType=PackingError,tolerance=1"));

        user1.set("packing-error-factor", 1.);
        CounterBinary strict(param1, param2);
        counted(strict);
        EXPECT_EQUAL(strict.check(), "\n* maximum difference (1) greater than 0.5");
    }


    SECTION("check requires values") {
        param::SimpleParametrisation param;
        CounterBinary counter(param, param);

        counter.reset(field1, field2);
        EXPECT_THROWS_AS(counter.check(), eckit::AssertionFailed);
    }
}


}  // namespace mir::stats::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
