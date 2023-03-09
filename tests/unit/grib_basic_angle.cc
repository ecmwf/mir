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
#include <vector>

#include "eckit/testing/Test.h"
#include "eckit/types/Fraction.h"

#include "mir/grib/BasicAngle.h"
#include "mir/util/BoundingBox.h"
#include "mir/util/Increments.h"


namespace mir::tests::unit {


CASE("lcm") {
    using grib::lcm;

    EXPECT_EQUAL(2 * 2 * 3 * 3, lcm(2 * 2 * 3, 2 * 3 * 3));
    EXPECT_EQUAL(3 * 3 * 5 * 5, lcm(3 * 3 * 5, 3 * 5 * 5));

    EXPECT_EQUAL(60, lcm(2 * 3, 3 * 4, 4 * 5));
    EXPECT_EQUAL(120, lcm(2 * 3 * 4, 3 * 4 * 5, 4 * 5 * 6));
    EXPECT_EQUAL(840, lcm(2 * 3 * 4, 3 * 4 * 5, 4 * 5 * 6, 5 * 6 * 7));

    EXPECT_EQUAL(0, lcm(1, 0));
    EXPECT_EQUAL(0, lcm(0, 1));
    EXPECT_EQUAL(0, lcm(0, 0));
}


CASE("gcd") {
    using grib::gcd;

    EXPECT_EQUAL(2 * 3, gcd(2 * 2 * 3, 2 * 3 * 3));
    EXPECT_EQUAL(3 * 5, gcd(3 * 3 * 5, 3 * 5 * 5));

    EXPECT_EQUAL(2, gcd(2 * 3, 3 * 4, 4 * 5));
    EXPECT_EQUAL(12, gcd(2 * 3 * 4, 3 * 4 * 5, 4 * 5 * 6));
    EXPECT_EQUAL(6, gcd(2 * 3 * 4, 3 * 4 * 5, 4 * 5 * 6, 5 * 6 * 7));

    EXPECT_EQUAL(1, gcd(1, 0));
    EXPECT_EQUAL(2, gcd(0, 2));
    EXPECT_EQUAL(0, gcd(0, 0));
}


CASE("Climate files <= v020 (1km)") {
    using grib::Fraction;

    auto fraction_via_double = [](const Fraction& f) {
        return eckit::Fraction(static_cast<double>(f.num) / static_cast<double>(f.den));
    };

    auto fraction_via_fraction = [](const Fraction& f) {
        return eckit::Fraction(static_cast<eckit::Fraction::value_type>(f.num),
                               static_cast<eckit::Fraction::value_type>(f.den));
    };


    // GRIB to value
    const Fraction fractions[] = {{2, 240},       // iDirectionIncrementInDegrees
                                  {2, 240},       // jDirectionIncrementInDegrees
                                  {21599, 240},   // latitudeOfFirstGridPointInDegrees
                                  {1, 240},       // longitudeOfFirstGridPointInDegrees
                                  {-21599, 240},  // latitudeOfLastGridPointInDegrees
                                  {86399, 240}};  // longitudeOfLastGridPointInDegrees

    for (const auto& f : fractions) {
        EXPECT_EQUAL(fraction_via_double(f), fraction_via_fraction(f));
    }


    // basic angle
    grib::BasicAngle basic(fractions[0], fractions[1], fractions[2], fractions[3], fractions[4], fractions[5]);

    EXPECT_EQUAL(basic.num, 1L);
    EXPECT_EQUAL(basic.den, 240L);


    // bounding box/increments normalised numerators
    const util::Increments inc(fraction_via_fraction(fractions[0]), fraction_via_fraction(fractions[1]));

    const util::BoundingBox bbox(fraction_via_fraction(fractions[2]), fraction_via_fraction(fractions[3]),
                                 fraction_via_fraction(fractions[4]), fraction_via_fraction(fractions[5]));

    struct test_t {
        explicit test_t(const Longitude& lon, const long _num) : fraction(lon.fraction()), num(_num) {}
        explicit test_t(const Latitude& lat, const long _num) : fraction(lat.fraction()), num(_num) {}
        const Fraction fraction;
        const long num;
    };

    for (const auto& test : {
             test_t{inc.west_east().longitude(), 2},
             test_t{inc.south_north().latitude(), 2},
             test_t{bbox.north(), 21599},
             test_t{bbox.west(), 1},
             test_t{bbox.south(), -21599},
             test_t{bbox.east(), 86399},
         }) {
        EXPECT_EQUAL(basic.numerator(test.fraction), test.num);
    }
}


}  // namespace mir::tests::unit


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
