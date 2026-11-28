// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <cmath>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "eckit/geo/Grid.h"
#include "eckit/testing/Test.h"
#include "eckit/types/Fraction.h"

#include "mir/iterator/GridIterator.h"
#include "mir/iterator/UnstructuredIterator.h"
#include "mir/iterator/detail/RegularIterator.h"
#include "mir/util/BoundingBox.h"
#include "mir/util/Increments.h"

#include "repres.h"


namespace mir::repres::tests {


using eckit::Fraction;
using iterator::detail::RegularIterator;


CASE("RegularIterator") {
    SECTION("range of a bounding box (MIR-390)") {
        // 89.9958333333, 0.004166666667, -89.9958333333, 359.995833333
        const util::BoundingBox bbox{Fraction(21599, 240), Fraction(1, 240), Fraction(-21599, 240),
                                     Fraction(86355, 240)};
        const PointLatLon reference{bbox.south(), bbox.west()};

        const size_t Ni = 43200;
        const size_t Nj = 21600;

        const util::Increments inc((bbox.east().fraction() - bbox.west().fraction()) / Ni,
                                   (bbox.north().fraction() - bbox.south().fraction()) / Nj);

        RegularIterator lat{bbox.south().fraction(), bbox.north().fraction(), inc.south_north().latitude().fraction(),
                            reference.lat().fraction()};

        EXPECT(lat.a() == bbox.south().fraction());
        EXPECT(lat.b() == bbox.north().fraction());
        EXPECT_EQUAL(lat.n(), Nj + 1);
        EXPECT(lat.a() + (lat.n() - 1) * lat.inc() == lat.b());

        RegularIterator lon{bbox.west().fraction(), bbox.east().fraction(), inc.west_east().longitude().fraction(),
                            reference.lon().fraction(), Longitude::GLOBE.fraction()};

        EXPECT(lon.a() == bbox.west().fraction());
        EXPECT(lon.b() == bbox.east().fraction());
        EXPECT_EQUAL(lon.n(), Ni + 1);
        EXPECT(lon.a() + (lon.n() - 1) * lon.inc() == lon.b());

        EXPECT(bbox == util::BoundingBox(lat.b(), lon.a(), lat.a(), lon.b()));
    }


    SECTION("shifted by the reference") {
        RegularIterator it{Fraction(0), Fraction(10), Fraction(2), Fraction(1)};
        EXPECT(it.a() == 1);
        EXPECT(it.b() == 9);
        EXPECT_EQUAL(it.n(), 5);
    }


    SECTION("zero increment, or a single value") {
        RegularIterator zero{Fraction(1), Fraction(5), Fraction(0), Fraction(0)};
        EXPECT(zero.a() == 1);
        EXPECT(zero.b() == 1);
        EXPECT_EQUAL(zero.n(), 1);

        RegularIterator single{Fraction(2), Fraction(2), Fraction(1), Fraction(0)};
        EXPECT(single.a() == 2);
        EXPECT(single.b() == 2);
        EXPECT_EQUAL(single.n(), 1);
    }


    SECTION("periodic (not repeating the first value)") {
        RegularIterator periodic{Fraction(0), Fraction(360), Fraction(1), Fraction(0), Fraction(360)};
        EXPECT(periodic.a() == 0);
        EXPECT(periodic.b() == 359);
        EXPECT_EQUAL(periodic.n(), 360);

        RegularIterator not_periodic{Fraction(0), Fraction(359), Fraction(1), Fraction(0), Fraction(360)};
        EXPECT(not_periodic.b() == 359);
        EXPECT_EQUAL(not_periodic.n(), 360);
    }


    SECTION("adjust (up or down, to a multiple of the increment)") {
        EXPECT(RegularIterator::adjust(Fraction(5, 2), Fraction(1), true) == 3);
        EXPECT(RegularIterator::adjust(Fraction(5, 2), Fraction(1), false) == 2);
        EXPECT(RegularIterator::adjust(Fraction(-5, 2), Fraction(1), true) == -2);
        EXPECT(RegularIterator::adjust(Fraction(-5, 2), Fraction(1), false) == -3);
        EXPECT(RegularIterator::adjust(Fraction(2), Fraction(1), true) == 2);
        EXPECT(RegularIterator::adjust(Fraction(2), Fraction(1), false) == 2);
    }


    SECTION("MIR-351: bounds encoded to the microdegree") {
        // latitudes of 30/128 degree, half an increment off the equator (+-80.0390625), encoded to the microdegree
        const Fraction inc(30, 128);
        const Fraction s(-80.039062);
        const Fraction n(80.039062);

        // bounds within the precision of a grid point (truncated inwards) include it
        EXPECT(RegularIterator(s, n, inc, s).n() == 684);

        // ... and further from it don't
        EXPECT(RegularIterator(s, Fraction(80.039), inc, s).n() == 683);
    }


    SECTION("invalid") {
        EXPECT_THROWS_AS(RegularIterator(Fraction(2), Fraction(1), Fraction(1), Fraction(0)), eckit::AssertionFailed);
        EXPECT_THROWS_AS(RegularIterator(Fraction(0), Fraction(1), Fraction(-1), Fraction(0)), eckit::AssertionFailed);
        EXPECT_THROWS_AS(RegularIterator::adjust(Fraction(1), Fraction(0), true), eckit::AssertionFailed);
    }
}


CASE("Iterator: points (rotated, unrotated, 3D), index, end") {
    RepresentationHandle repres(make("{grid: [10, 10], area: [10, -10, -10, 10], rotation: [-40, 22]}"));
    std::unique_ptr<Iterator> it(repres->iterator());

    size_t n = 0;
    while (it->next()) {
        EXPECT_EQUAL(it->index(), n++);
        EXPECT(*it);

        // the rotated point is geographic, the unrotated point is in the grid frame
        const auto& p = it->pointUnrotated();
        const auto& q = it->pointRotated();
        EXPECT(-10. <= p.lat().value() && p.lat().value() <= 10.);
        EXPECT_NOT(same(q.lat(), p.lat().value(), 1e-3) && same(q.lon(), p.lon().value(), 1e-3));

        const auto r = it->point3D();
        const auto s = Iterator::point_3D(q);
        EXPECT(eckit::geo::points_equal(r, s));
    }

    EXPECT_EQUAL(n, 9);
    EXPECT_NOT(*it);

    // past the end
    EXPECT_THROWS_AS(it->pointUnrotated(), eckit::AssertionFailed);
    EXPECT_THROWS_AS(it->pointRotated(), eckit::AssertionFailed);
    EXPECT_THROWS_AS(it->point3D(), eckit::AssertionFailed);
    EXPECT_THROWS_AS(it->next(), eckit::AssertionFailed);

    // the north pole, on the axis
    const auto pole = Iterator::point_3D(PointLonLat{0., 90.});
    EXPECT(same(pole.X(), 0., 1e-6) && same(pole.Y(), 0., 1e-6) && pole.Z() > 0.);
}


CASE("GridIterator: order of the points (as stored, or canonical)") {
    std::unique_ptr<const eckit::geo::Grid> grid(
        eckit::geo::GridFactory::make_from_string("{grid: [10, 10], area: [10, -10, -10, 10], order: i-j+}"));

    iterator::GridIterator a(*grid);
    iterator::GridIterator b(*grid, true);

    Iterator& stored    = a;
    Iterator& canonical = b;

    // as stored: south-east first; canonical: north-west first
    ASSERT(stored.next());
    EXPECT(same(stored.pointUnrotated().lat().value(), -10.));
    EXPECT(same(stored.pointUnrotated().lon().value(), 10.));

    ASSERT(canonical.next());
    EXPECT(same(canonical.pointUnrotated().lat().value(), 10.));
    EXPECT(same(canonical.pointUnrotated().lon().value(), -10.));

    std::ostringstream str;
    str << canonical;
    EXPECT(str.str().find("GridIterator[") == 0);
    EXPECT(str.str().find("index=0") != std::string::npos);
}


CASE("UnstructuredIterator") {
    const std::vector<double> latitudes{10., 20., 30.};
    const std::vector<double> longitudes{1., 2., 3.};

    iterator::UnstructuredIterator unstructured(latitudes, longitudes);
    Iterator& it = unstructured;

    size_t n = 0;
    while (it.next()) {
        EXPECT_EQUAL(it.index(), n);
        EXPECT_EQUAL(it.pointUnrotated().lat().value(), latitudes[n]);
        EXPECT_EQUAL(it.pointUnrotated().lon().value(), longitudes[n]);
        ++n;
    }
    EXPECT_EQUAL(n, 3);

    std::ostringstream str;
    str << it;
    EXPECT(str.str().find("UnstructuredGridIterator[") == 0);

    EXPECT_THROWS_AS(iterator::UnstructuredIterator({1., 2.}, {1.}), eckit::AssertionFailed);
}


}  // namespace mir::repres::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
