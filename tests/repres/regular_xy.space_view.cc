// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <cmath>
#include <string>
#include <vector>

#include "eckit/geo/Point.h"
#include "eckit/testing/Test.h"

#include "regular_xy.h"
#include "repres.h"


namespace mir::repres::tests {


CASE("space_view (GRIB): points as the scanning, element diagonal") {
    // sectors of GRIB2 space_view (Nr=6.6107, dx=3622), points as the scanning (first, second and last)
    const keys_t sector{{"Nx", 16},      {"Ny", 16},   {"dx", 3622}, {"dy", 3622},   {"Xp", 1856000},
                        {"Yp", 1856000}, {"Xo", 1500}, {"Yo", 1700}, {"Nr", 6610700}};

    struct test_t {
        std::string name;
        keys_t keys;
        points_t points;
    };

    const std::vector<test_t> tests{
        {"shapeOfTheEarth=6",
         {{"shapeOfTheEarth", 6}},
         {{4.223693723, 350.299524805}, {4.223622147, 350.327193534}, {3.815601110, 350.719698129}}},
        {"shapeOfTheEarth=6, iScansNegatively, jScansPositively",
         {{"shapeOfTheEarth", 6}, {"iScansNegatively", 1}, {"jScansPositively", 1}},
         {{-4.223693724, 9.700475199}, {-4.223622148, 9.672806469}, {-3.815601111, 9.280301874}}},
        {"shapeOfTheEarth=5, sub-satellite point",
         {{"shapeOfTheEarth", 5},
          {"longitudeOfSubSatellitePoint", -75000000},
          {"Ny", 12},
          {"dy", 3610},
          {"Xo", 2400},
          {"Yo", 1200}},
         {{18.425011761, 300.905565350}, {18.425558651, 300.936080607}, {18.106822780, 301.327135202}}},
    };

    for (const auto& test : tests) {
        SECTION(test.name) {
            RepresentationHandle repres(representation(message("space_view", sector + test.keys)));

            const auto visited = points(*repres);
            EXPECT_EQUAL(visited.size(), repres->numberOfPoints());

            const std::vector<point_t> cases{visited.front(), visited[1], visited.back()};
            for (size_t i = 0; i < cases.size(); ++i) {
                const auto& p = cases[i];
                const auto& q = test.points[i];

                // longitudes are fractions (mir::Longitude), which approximate (to about 1e-5 degrees)
                EXPECT(std::abs(p.lat - q.lat) < 1e-6);
                EXPECT(std::abs(std::remainder(p.lon - q.lon, 360.)) < 1e-5);
            }

            double d = 0.;
            EXPECT(repres->getLongestElementDiagonal(d));
            EXPECT(d > 0.);

            grib_info info;
            EXPECT_THROWS_AS(repres->fillGrib(info), exception::FunctionalityNotSupported);
        }
    }
}


}  // namespace mir::repres::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
