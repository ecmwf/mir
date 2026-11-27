// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "eckit/geo/Grid.h"
#include "eckit/testing/Test.h"

#include "mir/iterator/GridIterator.h"
#include "mir/iterator/UnstructuredIterator.h"

#include "repres.h"


namespace mir::repres::tests {


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
