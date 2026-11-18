// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <algorithm>
#include <memory>
#include <numeric>
#include <string>
#include <vector>

#include "eckit/geo/Grid.h"
#include "eckit/testing/Test.h"

#include "mir/key/grid/Grid.h"
#include "mir/repres/gauss/Gaussian.h"
#include "mir/util/BoundingBox.h"
#include "mir/util/Domain.h"
#include "mir/util/MeshGeneratorParameters.h"
#include "mir/util/Rotation.h"

#include "repres.h"


namespace mir::repres::tests {


using util::BoundingBox;


CASE("Gaussian latitudes and quadrature weights") {
    for (size_t N : {1, 2, 8, 64}) {
        const auto& lats = Gaussian::latitudes(N);
        const auto& w    = Gaussian::weights(N);

        EXPECT_EQUAL(lats.size(), 2 * N);
        EXPECT_EQUAL(w.size(), 2 * N);

        EXPECT(std::is_sorted(lats.begin(), lats.end(), [](double a, double b) { return a > b; }));
        EXPECT(same(lats.front(), -lats.back()));
        EXPECT(0. < lats.front() && lats.front() < 90.);

        EXPECT(same(std::accumulate(w.begin(), w.end(), 0.), 1., 1e-12));
    }
}


CASE("Gaussian grids: number of points and first points, global and cropped") {
    struct test_t {
        std::string grid;
        size_t numberOfPoints;
        size_t numberOfCroppedPoints;
        BoundingBox bbox;
        Point2 globalP1;
        Point2 globalP2;
        Point2 localP1;
        Point2 localP2;
    };

    const std::vector<test_t> tests{
        {"F128",
         131072,
         858,
         {80.351, -150.46875, 79.648, 150.46875},
         {89.462821568577, 0},
         {89.462821568577, 0.703125},
         {80.350237152036, -150.46875},
         {80.350237152036, -149.765625}},
        {"F128",
         131072,
         429,
         {80.350, -150.46875, 79.648, 150.46875},
         {89.462821568577, 0},
         {89.462821568577, 0.703125},
         {79.648526993666, -150.46875},
         {79.648526993666, -149.765625}},
        {"F320",
         819200,
         912,
         {56, -16.5, 50.5, -3},
         {89.784876907219, 0},
         {89.784876907219, 0.28125},
         {55.784517651898, -16.3125},
         {55.784517651898, -16.03125}},
        {"F640",  // MIR-555
         3276800,
         2400,
         {54., -16.5, 50.5, -3.},
         {89.89239644558999, 0.},
         {89.89239644558999, 0.140625},
         {53.908623449839, -16.453125},
         {53.908623449839, -16.3125}},
        {"F1280",
         13107200,
         910080,
         {75, -30, 25, 60},
         {89.946187715666, 0},
         {89.946187715666, 0.0703125},
         {74.97363389192699, -29.953125},
         {74.97363389192699, -29.8828125}},
        {"O320",
         421120,
         16133,
         {58, -130, 25, -60},
         {89.784876907219, 0},
         {89.784876907219, 18},
         {57.751728769309, -129.327731092437},
         {57.751728769309, -128.5714285714286}},
        {"O320",
         421120,
         7511,
         {-25, 135, -47, 179},
         {89.784876907219, 0},
         {89.784876907219, 18},
         {-25.152216627037, 135.1914893617021},
         {-25.152216627037, 135.5744680851064}},
        {"O320",
         421120,
         28101,
         {12, -81, -50, -35},
         {89.784876907219, 0},
         {89.784876907219, 18},
         {11.943790218098, -80.74468085106383},
         {11.943790218098, -80.42553191489361}},
        {"O320",
         421120,
         9877,
         {44, 91, 21, 143},
         {89.784876907219, 0},
         {89.784876907219, 18},
         {43.981247789094, 91.07142857142857},
         {43.981247789094, 91.60714285714286}},
        {"O320",
         421120,
         8743,
         {60, 23, 35, 80},
         {89.784876907219, 0},
         {89.784876907219, 18},
         {59.999969762058, 23.51351351351351},
         {59.999969762058, 24.32432432432432}},
        {"O640",
         1661440,
         4512,
         {52, 7, 43, 27.7},
         {89.89239644558999, 0},
         {89.89239644558999, 18},
         {51.940642605563, 7.2},
         {51.940642605563, 7.527272727272727}},
        {"O640",
         1661440,
         63842,
         {58, -130, 25, -60},
         {89.89239644558999, 0},
         {89.89239644558999, 18},
         {57.985155059998, -129.9568965517241},
         {57.985155059998, -129.5689655172414}},
        {"O640",
         1661440,
         29674,
         {-25, 135, -47, 179},
         {89.89239644558999, 0},
         {89.89239644558999, 18},
         {-25.091758985537, 135},
         {-25.091758985537, 135.1931330472103}},
        {"O1280",
         6599680,
         442460,
         {12, -81, -50, -35},
         {89.946187715666, 0},
         {89.946187715666, 18},
         {11.985940014128, -80.95152603231598},
         {11.985940014128, -80.87073608617594}},
        {"O1280",
         6599680,
         12370,
         {37.6025, -114.8907, 27.7626, -105.1875},
         {89.946187715666, 0},
         {89.946187715666, 18},
         {37.574691602387, -114.84},
         {37.574691602387, -114.72}},
        {"O1280",
         6599680,
         6599646,
         {90, 0, -90, 359.929},
         {89.946187715666, 0},
         {89.946187715666, 18},
         {89.946187715666, 0},
         {89.946187715666, 18}},
        {"O1280",
         6599680,
         118210,
         {-25, 135, -47, 179},
         {89.946187715666, 0},
         {89.946187715666, 18},
         {-25.061510912726, 135},
         {-25.061510912726, 135.0969827586207}},
        {"N128",
         88838,
         88838,
         {90, 0, -90, 359.297},
         {89.462821568577, 0},
         {89.462821568577, 20},
         {89.462821568577, 0},
         {89.462821568577, 20}},
        {"N320",
         542080,
         13437,
         {-60, 50, -90, 180},
         {89.784876907219, 0},
         {89.784876907219, 20},
         {-60.280999861572, 50.13333333333333},
         {-60.280999861572, 50.66666666666666}},
        {"N640",
         2140702,
         52654,
         {-60, 50, -90, 180},
         {89.89239644558999, 0},
         {89.89239644558999, 20},
         {-60.093705799122, 50.13333333333333},
         {-60.093705799122, 50.4}},
    };

    const BoundingBox globe;

    auto first_two_points = [](const Representation& repres, const BoundingBox& bbox, const Point2& p1,
                               const Point2& p2) {
        std::unique_ptr<Iterator> it(repres.iterator());

        ASSERT(it->next());
        EXPECT(bbox.contains(*(*it)));
        EXPECT(eckit::geometry::points_equal(*(*it), p1));

        ASSERT(it->next());
        EXPECT(bbox.contains(*(*it)));
        EXPECT(eckit::geometry::points_equal(*(*it), p2));
    };

    for (const auto& test : tests) {
        RepresentationHandle global(key::grid::Grid::lookup(test.grid).representation());
        EXPECT_EQUAL(global->numberOfPoints(), test.numberOfPoints);
        first_two_points(*global, globe, test.globalP1, test.globalP2);

        RepresentationHandle cropped(global->croppedRepresentation(test.bbox));
        EXPECT_EQUAL(cropped->numberOfPoints(), test.numberOfCroppedPoints);
        first_two_points(*cropped, test.bbox, test.localP1, test.localP2);
    }
}


CASE("Gaussian grids by name (classic, octahedral and regular), unrotated and rotated") {
    const util::Rotation rotation(-40., 22.);

    struct test_t {
        std::string name;
        size_t numberOfPoints;
        bool sameAsImplemented;
    };

    const std::vector<test_t> tests{
        {"N16", 0, false},  // (number of points not checked)
        {"O8", 2 * (20 + 24 + 28 + 32 + 36 + 40 + 44 + 48), false},
        {"F8", 32 * 16, true},
    };

    for (const auto& test : tests) {
        SECTION(test.name) {
            const auto& grid = key::grid::Grid::lookup(test.name);
            RepresentationHandle unrotated(grid.representation());
            RepresentationHandle rotated(grid.representation(rotation));

            const auto n = unrotated->numberOfPoints();
            if (test.numberOfPoints > 0) {
                EXPECT_EQUAL(n, test.numberOfPoints);
            }

            EXPECT_EQUAL(rotated->numberOfPoints(), n);
            EXPECT_EQUAL(count(*unrotated), n);
            EXPECT_EQUAL(count(*rotated), n);
            EXPECT_EQUAL(static_cast<size_t>(unrotated->atlasGrid().size()), n);
            EXPECT_EQUAL(static_cast<size_t>(rotated->atlasGrid().size()), n);

            EXPECT(unrotated->uniqueName().find(test.name) == 0);
            EXPECT(rotated->uniqueName().find(test.name) == 0);
            EXPECT(rotated->uniqueName() != unrotated->uniqueName());
            EXPECT(to_string(*rotated).find("rotation") != std::string::npos);
            EXPECT(to_string(*unrotated).find("N=" + test.name.substr(1)) != std::string::npos);

            // the same grid, by name
            RepresentationHandle again(grid.representation());
            EXPECT(unrotated->sameAs(*again));

            if (test.sameAsImplemented) {
                EXPECT_NOT(rotated->sameAs(*unrotated));
                EXPECT_NOT(unrotated->sameAs(*rotated));
            }
            else {
                EXPECT_THROWS_AS(rotated->sameAs(*unrotated), eckit::NotImplemented);
            }

            // GRIB
            grib_info info;
            rotated->fillGrib(info);
            EXPECT(info.grid.grid_type == CODES_UTIL_GRID_SPEC_ROTATED_GG ||
                   info.grid.grid_type == CODES_UTIL_GRID_SPEC_REDUCED_ROTATED_GG);
            EXPECT(same(info.grid.latitudeOfSouthernPoleInDegrees, -40.));
            EXPECT(same(info.grid.longitudeOfSouthernPoleInDegrees, 22.));

            // the spec (of the rotated grid) describes the same grid
            std::unique_ptr<const eckit::geo::Grid> spec(
                eckit::geo::GridFactory::make_from_string(rotated->spec().str()));
            EXPECT_EQUAL(spec->size(), n);

            // cropped (in the rotated frame), and the geographic domain
            RepresentationHandle cropped(rotated->croppedRepresentation({30, -30, -30, 30}));
            EXPECT(cropped->numberOfPoints() < n);
            EXPECT_NOT(cropped->geographicDomain() == cropped->domain());
            EXPECT(rotated->geographicDomain().isGlobal());

            util::MeshGeneratorParameters mesh;
            unrotated->fillMeshGen(mesh);
            EXPECT_EQUAL(mesh.meshGenerator_, "structured");
            const bool octahedral = test.name == "O8";
            EXPECT_EQUAL(mesh.getBool("triangulate"), octahedral);
        }
    }
}


}  // namespace mir::repres::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
