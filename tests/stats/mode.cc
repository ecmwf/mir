// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <cmath>
#include <sstream>
#include <vector>

#include "eckit/exception/Exceptions.h"
#include "eckit/testing/Test.h"

#include "mir/param/SimpleParametrisation.h"
#include "mir/stats/detail/ModeT.h"

#include "stats.h"


namespace mir::stats::tests {


template <typename T>
std::string printed(const T& stats) {
    std::ostringstream str;
    stats.print(str);
    return str.str();
}


CASE("ModeIntegral") {
    SECTION("most frequent (rounded) value, ties by disambiguation") {
        detail::ModeIntegral max(true);
        detail::ModeIntegral min(false);

        for (const auto& v : std::vector<double>{0.9, 1.2, 2., 3.6, 4.4}) {
            max(v);
            min(v);
        }

        EXPECT_EQUAL(max.mode(), 4.);
        EXPECT_EQUAL(min.mode(), 1.);
        EXPECT_EQUAL(printed(max), "Mode[mode=4]");

        min(2.f);
        min(2);
        EXPECT_EQUAL(min.mode(), 2.);
    }


    SECTION("merge, reset and empty") {
        detail::ModeIntegral a(true);
        detail::ModeIntegral b(true);
        EXPECT(std::isnan(a.mode()));

        a(1);
        a(1);
        b(7);
        b(7);
        b(7);
        EXPECT_EQUAL(a.mode(), 1.);

        a += b;
        EXPECT_EQUAL(a.mode(), 7.);

        a.reset();
        EXPECT(std::isnan(a.mode()));
    }


    SECTION("by parametrisation (disambiguating to the maximum by default)") {
        param::SimpleParametrisation param;
        detail::ModeIntegral max(param);

        param.set("mode-disambiguate-max", false);
        detail::ModeIntegral min(param);

        for (const auto& v : std::vector<int>{3, 5}) {
            max(v);
            min(v);
        }

        EXPECT_EQUAL(max.mode(), 5.);
        EXPECT_EQUAL(min.mode(), 3.);
    }
}


CASE("MedianIntegral") {
    struct test_t {
        std::vector<int> values;
        double medianMax;
        double medianMin;
    };

    for (const auto& test : std::vector<test_t>{{{5}, 5., 5.},
                                                {{1, 2, 3, 4}, 3., 2.},
                                                {{1, 1, 2, 4, 4}, 2., 2.},
                                                {{1, 9, 9}, 9., 9.},
                                                {{9, 1, 1}, 1., 1.},
                                                {{1, 2, 2, 3, 4, 7, 9}, 3., 3.},
                                                {{22, 1, 3, 6, 6, 6, 6, 7, 7, 12, 12}, 6., 6.}}) {
        detail::MedianIntegral max(true);
        detail::MedianIntegral min(false);

        for (const auto& v : test.values) {
            max(v);
            min(v);
        }

        EXPECT_EQUAL(max.median(), test.medianMax);
        EXPECT_EQUAL(min.median(), test.medianMin);
    }

    detail::MedianIntegral a(true);
    detail::MedianIntegral b(true);
    EXPECT(std::isnan(a.median()));

    a(1.);
    b(5.);
    b(6.);
    a += b;
    EXPECT_EQUAL(a.median(), 5.);
    EXPECT_EQUAL(printed(a), "Median[median=5]");
}


CASE("ModeReal") {
    SECTION("most frequent bin, values by bin") {
        detail::ModeReal mode(true, {1., 2., 3.}, {1.5, 2.5});
        EXPECT(std::isnan(mode.mode()));

        // bins are ]-inf, 1.5], ]1.5, 2.5], ]2.5, inf[
        for (const auto& v : std::vector<double>{-10., 1.5, 1.6, 2.5, 99.}) {
            mode(v);
        }

        EXPECT_EQUAL(mode.mode(), 2.);
        EXPECT_EQUAL(printed(mode), "Mode[mode=2,values=[1,2,3],mins=[1.5,2.5]]");

        detail::ModeReal other(true, {1., 2., 3.}, {1.5, 2.5});
        other(3.f);
        other(3);
        other(3.);
        mode += other;
        EXPECT_EQUAL(mode.mode(), 3.);

        mode.reset();
        EXPECT(std::isnan(mode.mode()));
    }


    SECTION("default bins (0 and 1, split at 0.5), ties by disambiguation") {
        detail::ModeReal max(true);
        detail::ModeReal min(false);

        for (const auto& v : std::vector<double>{0.2, 0.7}) {
            max(v);
            min(v);
        }

        EXPECT_EQUAL(max.mode(), 1.);
        EXPECT_EQUAL(min.mode(), 0.);
    }


    SECTION("by parametrisation") {
        param::SimpleParametrisation param;
        param.set("mode-real-values", std::vector<double>{4., 5.});
        param.set("mode-real-min", std::vector<double>{4.5});

        detail::ModeReal mode(param);
        mode(4.);
        mode(4.6);
        mode(4.7);
        EXPECT_EQUAL(mode.mode(), 5.);
    }


    SECTION("values and limits are interleaved and sorted") {
        EXPECT_THROWS_AS(detail::ModeReal(true, {1., 2.}, {}), eckit::AssertionFailed);
        EXPECT_THROWS_AS(detail::ModeReal(true, {1., 2.}, {3.}), eckit::AssertionFailed);
        EXPECT_THROWS_AS(detail::ModeReal(true, {2., 1.}, {1.5}), eckit::AssertionFailed);
        EXPECT_THROWS_AS(detail::ModeReal(true, {1., 2., 3.}, {2.5, 1.5}), eckit::AssertionFailed);
        EXPECT_NO_THROW(detail::ModeReal(true, {1.}, {}));
    }
}


CASE("ModeBoolean") {
    detail::ModeBoolean max(true);
    detail::ModeBoolean min(false, 0.5);
    EXPECT(std::isnan(max.mode()));

    max(0.);
    min(0.);
    max(0.5);
    min(0.5);

    // tie
    EXPECT_EQUAL(max.mode(), 1.);
    EXPECT_EQUAL(min.mode(), 0.);
    EXPECT_EQUAL(printed(max), "Mode[mode=1,majority=0]");

    min(0.1);
    EXPECT_EQUAL(min.mode(), 0.);

    detail::ModeBoolean ones(false);
    ones(1.);
    ones(1.);
    min += ones;
    EXPECT_EQUAL(min.mode(), 1.);
    EXPECT_EQUAL(min.majority_, 1);

    min.reset();
    EXPECT(std::isnan(min.mode()));

    param::SimpleParametrisation param;
    param.set("mode-disambiguate-max", false);
    param.set("mode-boolean-min", 10.);

    detail::ModeBoolean byParam(param);
    byParam(9.);
    byParam(10.);
    EXPECT_EQUAL(byParam.mode(), 0.);
    byParam(11.);
    EXPECT_EQUAL(byParam.mode(), 1.);

    // defaults: threshold 0.5, ties as 1
    detail::ModeBoolean byDefault(param::SimpleParametrisation{});
    byDefault(0.4);
    EXPECT_EQUAL(byDefault.mode(), 0.);
    byDefault(0.6);
    EXPECT_EQUAL(byDefault.mode(), 1.);
}


}  // namespace mir::stats::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
