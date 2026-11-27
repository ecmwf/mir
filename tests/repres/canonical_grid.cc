// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <numeric>
#include <string>
#include <variant>
#include <vector>

#include "eckit/geo/Grid.h"
#include "eckit/geo/figure/Earth.h"
#include "eckit/testing/Test.h"

#include "mir/param/SimpleParametrisation.h"
#include "mir/util/LonLatToXYZ.h"

#include "repres.h"


namespace mir::repres::tests {


CASE("canonicalGrid: rotated points, and of the grid frame (unrotated)") {
    RepresentationHandle repres(make("{grid: [10, 10], area: [10, -10, -10, 10], rotation: [-40, 22]}"));

    const auto& rotated   = repres->canonicalGrid();
    const auto& unrotated = repres->canonicalGridUnrotated();
    EXPECT(&rotated != &unrotated);
    EXPECT_EQUAL(rotated.size(), 9);
    EXPECT_EQUAL(unrotated.size(), 9);

    size_t n = 0;
    for (auto it = rotated.cbegin(), jt = unrotated.cbegin(); it; ++it, ++jt, ++n) {
        EXPECT_EQUAL(it.index(), n);
        EXPECT_EQUAL(jt.index(), n);

        // the rotated point is geographic, the unrotated point is of the grid frame
        const auto p = std::get<PointLonLat>(*jt);
        const auto q = std::get<PointLonLat>(*it);
        EXPECT(-10. <= p.lat() && p.lat() <= 10.);
        EXPECT_NOT(same(q.lat(), p.lat(), 1e-3) && same(q.lon(), p.lon(), 1e-3));
    }

    EXPECT_EQUAL(n, 9);

    // the same grid (cached)
    EXPECT(&rotated == &repres->canonicalGrid());
    EXPECT(&unrotated == &repres->canonicalGridUnrotated());
}


CASE("canonicalGrid: the order of the values as held") {
    // the values of regular lat/lon grids are held in the default order (see reorderToCanonical)
    RepresentationHandle latlon(make("{grid: [10, 10], area: [10, -10, -10, 10], order: i-j+}"));
    EXPECT(latlon->grid().order() == "i-j+");
    EXPECT(latlon->canonicalGrid().order() == "i+j-");
    EXPECT(&latlon->canonicalGrid() == &latlon->canonicalGridUnrotated());

    // as encoded: south-east first; as held: north-west first
    const auto encoded = std::get<PointLonLat>(latlon->grid().first_point());
    EXPECT(same(encoded.lat(), -10.) && same(encoded.lon(), 10.));

    const auto held = std::get<PointLonLat>(latlon->canonicalGrid().first_point());
    EXPECT(same(held.lat(), 10.) && same(held.lon(), -10.));

    // the values of other grids are held as the grid
    RepresentationHandle healpix(make("{grid: H2, order: nested}"));
    EXPECT(&healpix->canonicalGrid() == &healpix->grid());
}


CASE("reorderToCanonical/reorderFromCanonical: the values of the points as held") {
    // (of the representations keeping an order, regular lat/lon and Gaussian grids)
    for (const std::string& gridspec :
         {"{grid: [10, 10], area: [10, -10, -10, 10], order: i-j+}", "{grid: F2, order: i-j+}"}) {
        RepresentationHandle repres(make(gridspec));
        const auto& encoded = repres->grid();
        const auto& held    = repres->canonicalGrid();
        EXPECT(encoded.order() != held.order());

        // values as encoded (the point index), as held: each value of its point
        MIRValuesVector values(encoded.size());
        std::iota(values.begin(), values.end(), 0.);

        repres->reorderToCanonical(values);
        for (size_t k = 0; k < values.size(); ++k) {
            const auto i = static_cast<size_t>(values[k]);
            EXPECT(eckit::geo::points_equal(held.point(k), encoded.point(i)));
        }

        repres->reorderFromCanonical(values);
        for (size_t k = 0; k < values.size(); ++k) {
            EXPECT_EQUAL(values[k], static_cast<double>(k));
        }
    }
}


CASE("canonicalGrid: unstructured points as given") {
    const std::vector<double> latitudes{10., 20., 30.};
    const std::vector<double> longitudes{1., 2., 3.};

    param::SimpleParametrisation param;
    param.set("gridType", "unstructured_grid");
    param.set("latitudes", latitudes);
    param.set("longitudes", longitudes);

    RepresentationHandle repres(RepresentationFactory::build(param));

    size_t n = 0;
    for (const auto& point : repres->canonicalGrid()) {
        const auto p = std::get<PointLonLat>(point);
        EXPECT_EQUAL(p.lat(), latitudes[n]);
        EXPECT_EQUAL(p.lon(), longitudes[n]);
        ++n;
    }
    EXPECT_EQUAL(n, 3);
}


CASE("LonLatToXYZ: on the figure, displaced from the poles") {
    RepresentationHandle repres(make("{grid: [10, 10], area: [10, -10, -10, 10], rotation: [-40, 22]}"));
    const auto& grid = repres->canonicalGrid();

    // the points as the grid's (eckit::geo::Search)
    const util::LonLatToXYZ to_xyz(grid.figure());
    const auto xyz = grid.to_xyz();

    size_t n = 0;
    for (const auto& point : grid) {
        const auto r = to_xyz(std::get<PointLonLat>(point));
        EXPECT(eckit::geo::points_equal(r, PointXYZ{xyz[0][n], xyz[1][n], xyz[2][n]}));
        ++n;
    }

    // the north pole, on the axis, unless displaced
    const auto pole = to_xyz(PointLonLat{0., 90.});
    EXPECT(same(pole.X(), 0., 1e-6) && same(pole.Y(), 0., 1e-6) && pole.Z() > 0.);

    const util::LonLatToXYZ displaced(eckit::geo::figure::Earth{}, 1e-3);
    const auto p = displaced(PointLonLat{0., 90.});
    EXPECT(p.X() > 1. && same(p.Y(), 0., 1e-6) && p.Z() < pole.Z());

    // a figure other than a sphere
    const auto& wgs84 = eckit::geo::figure::EARTH_WGS84;
    const util::LonLatToXYZ spheroid(wgs84);
    EXPECT(same(spheroid(PointLonLat{0., 0.}).X(), wgs84.a(), 1e-6));
    EXPECT(same(spheroid(PointLonLat{0., 90.}).Z(), wgs84.b(), 1e-6));
}


}  // namespace mir::repres::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
