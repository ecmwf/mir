// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <algorithm>
#include <memory>
#include <numeric>

#include "eckit/testing/Test.h"

#include "mir/data/MIRField.h"
#include "mir/repres/gauss/Gaussian.h"
#include "mir/repres/gauss/regular/RegularGG.h"
#include "mir/util/BoundingBox.h"
#include "mir/util/Domain.h"
#include "mir/util/MeshGeneratorParameters.h"

#include "repres.h"


namespace mir::repres::tests {


using util::BoundingBox;


CASE("Regular Gaussian grid") {
    using gauss::regular::RegularGG;

    RepresentationHandle global(new RegularGG(4));
    const auto& lats = Gaussian::latitudes(4);

    EXPECT_EQUAL(global->numberOfPoints(), 16 * 8);
    EXPECT(global->isGlobal());
    EXPECT_EQUAL(global->factory(), "regular_gg");
    EXPECT(global->uniqueName().find("F4") == 0);


    SECTION("fillGrib") {
        grib_info info;
        global->fillGrib(info);

        EXPECT_EQUAL(info.grid.grid_type, CODES_UTIL_GRID_SPEC_REGULAR_GG);
        EXPECT_EQUAL(info.grid.N, 4);
        EXPECT_EQUAL(info.grid.Ni, 16);
        EXPECT_EQUAL(info.grid.Nj, 8);
        EXPECT(same(info.grid.iDirectionIncrementInDegrees, 22.5));
        EXPECT(same(info.grid.latitudeOfFirstGridPointInDegrees, lats.front()));
        EXPECT(same(info.grid.longitudeOfFirstGridPointInDegrees, 0.));
        EXPECT(same(info.grid.latitudeOfLastGridPointInDegrees, lats.back()));
        EXPECT(same(info.grid.longitudeOfLastGridPointInDegrees, 337.5));
        EXPECT_EQUAL(info.grid.iScansNegatively, 0);
        EXPECT_EQUAL(info.grid.jScansPositively, 0);
    }


    SECTION("cropped (area corrected to the grid)") {
        RepresentationHandle cropped(new RegularGG(4, {50, 10, -50, 100}));
        EXPECT_EQUAL(cropped->numberOfPoints(), 4 * 4);
        EXPECT(cropped->boundingBox() == BoundingBox(lats[2], 22.5, lats[5], 90));
        EXPECT_NOT(cropped->isGlobal());
        EXPECT_NOT(cropped->sameAs(*global));

        RepresentationHandle same_crop(global->croppedRepresentation({50, 10, -50, 100}));
        EXPECT(same_crop->sameAs(*cropped));

        // a single latitude, or beyond the first/last latitudes
        RepresentationHandle band(new RegularGG(4, {lats[3], 0, lats[3], 90}));
        EXPECT_EQUAL(band->numberOfPoints(), 5);

        RepresentationHandle north(new RegularGG(4, {88, 0, 85, 90}));
        EXPECT_EQUAL(north->numberOfPoints(), 5);
        EXPECT(north->boundingBox().north() == Latitude(lats.front()));

        RepresentationHandle south(new RegularGG(4, {-85, 0, -88, 90}));
        EXPECT_EQUAL(south->numberOfPoints(), 5);
        EXPECT(south->boundingBox().south() == Latitude(lats.back()));
    }


    SECTION("extendBoundingBox (outwards, to the grid points)") {
        const BoundingBox bbox(10, 30, -10, 50);
        const auto extended = global->extendBoundingBox(bbox);

        EXPECT(extended.contains(bbox));
        EXPECT(extended == BoundingBox(lats[3], 0, lats[4], 67.5));

        // beyond the Gaussian latitudes (unchanged)
        const auto polar = global->extendBoundingBox({89, 30, -89, 50});
        EXPECT(polar.north() == Latitude(89));
        EXPECT(polar.south() == Latitude(-89));

        // almost the globe West-East
        const auto wide = global->extendBoundingBox({10, 1, -10, 350});
        EXPECT(wide.west() == Longitude::GREENWICH);
        EXPECT(wide.east() == Longitude::GLOBE);
    }


    SECTION("frame") {
        MIRValuesVector values(16 * 8, 1.);
        EXPECT_EQUAL(global->frame(values, 1, 9999.), 14 * 6);
        EXPECT_EQUAL(std::count(values.begin(), values.end(), 9999.), 14 * 6);
    }


    SECTION("meshing (poles)") {
        util::MeshGeneratorParameters mesh;
        global->fillMeshGen(mesh);
        EXPECT_EQUAL(mesh.meshGenerator_, "structured");
        EXPECT(mesh.getBool("force_include_north_pole"));
        EXPECT(mesh.getBool("force_include_south_pole"));
    }


    SECTION("angular precision") {
        RepresentationHandle exact(new RegularGG(4, {lats[1] + 1e-6, 0, lats[6] - 1e-6, 337.5}));
        RepresentationHandle approx(new RegularGG(4, {lats[1] - 1e-6, 0, lats[6] + 1e-6, 337.5}, 1e-5));

        EXPECT_EQUAL(exact->numberOfPoints(), 16 * 6);
        EXPECT_EQUAL(approx->numberOfPoints(), 16 * 6);
    }
}


CASE("Regular Gaussian grid: globalise (the points, then a global grid's points outside the domain)") {
    SECTION("rotated: the points, geographic") {
        RepresentationHandle regional(make("{grid: F8, area: [20, -20, -20, 20], rotation: [-40, 22]}"));
        const auto n = regional->numberOfPoints();

        MIRValuesVector values(n, 1.);
        data::MIRField field(regional, false, 9999.);
        field.update(values, 0);

        RepresentationHandle global(regional->globalise(field));
        ASSERT(global);
        EXPECT(n < global->numberOfPoints());

        std::unique_ptr<Iterator> a(regional->iterator());
        std::unique_ptr<Iterator> b(global->iterator());
        for (size_t k = 0; k < n; ++k) {
            ASSERT(a->next() && b->next());
            EXPECT(Point3::distance(a->point3D(), b->point3D()) < 1.);  // (metres, the unstructured grid approximately)
        }
    }

    SECTION("the values, then missing values") {
        RepresentationHandle regional(make("{grid: F8, area: [60, -10, 30, 40]}"));
        const auto n = regional->numberOfPoints();

        MIRValuesVector values(n);
        std::iota(values.begin(), values.end(), 0.);

        data::MIRField field(regional, false, 9999.);
        field.update(values, 0);

        RepresentationHandle global(regional->globalise(field));
        ASSERT(global);

        const auto size = global->numberOfPoints();
        EXPECT(n < size);
        EXPECT(field.hasMissing());

        // the values, then missing values
        const auto& v = field.values(0);
        EXPECT_EQUAL(v.size(), size);
        EXPECT(std::equal(values.begin(), values.end(), v.begin()));
        EXPECT(std::all_of(v.begin() + static_cast<std::ptrdiff_t>(n), v.end(), [](double x) { return x == 9999.; }));

        // the points of the domain are the representation's
        const auto dom = regional->domain();
        size_t inside  = 0;
        for (std::unique_ptr<Iterator> it(global->iterator()); it->next();) {
            if (dom.contains(it->pointUnrotated())) {
                EXPECT(it->index() < n);
                ++inside;
            }
        }
        EXPECT_EQUAL(inside, n);
    }
}


}  // namespace mir::repres::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
