// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <string>
#include <vector>

#include "eckit/testing/Test.h"
#include "eckit/types/FloatCompare.h"

#include "mir/data/MIRField.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/repres/latlon/RegularLL.h"
#include "mir/util/BoundingBox.h"
#include "mir/util/Domain.h"
#include "mir/util/Increments.h"
#include "mir/util/MeshGeneratorParameters.h"

#include "repres.h"


namespace mir::repres::tests {


using latlon::LatLon;
using latlon::RegularLL;
using util::BoundingBox;
using util::Increments;


CASE("RegularLL: (Ni, Nj) by increments and area, preserving the reference point") {
    struct test_t {
        std::string name;
        Increments increments;
        BoundingBox area;
        size_t Ni;
        size_t Nj;
    };

    const BoundingBox GLOBE;

    const std::vector<test_t> tests{
        {"001", Increments{7, 7}, {85, 0, -90, 357}, 52, 26},
        {"002", Increments{4, 4}, {90, 0, -90, 356}, 90, 46},
        {"003", Increments{2, 2}, {2, 0, 0, 2}, 2, 2},
        {"004", Increments{2, 2}, {2.1, 0, 0, 2.1}, 2, 2},
        {"005", Increments{2, 2}, {3, 1, 1, 3}, 2, 2},
        {"006", Increments{2, 2}, {37.6025, -114.8907, 27.7626, -105.1875}, 5, 5},
        {"007", Increments{2, 2}, {37.6025, -114.8907, 27.7626, -105.188}, 5, 5},
        {"008", Increments{2, 2}, {88, -178, -88, 180}, 180, 89},
        {"009", Increments{1, 1}, {0, -350, 0, 8}, 359, 1},
        {"010", Increments{0.5, 0.5}, {0, -350, 0, 9}, 719, 1},
        {"011", Increments{7, 7}, GLOBE, 52, 26},
        {"012", Increments{4, 4}, GLOBE, 90, 46},
        {"013", Increments{2, 2}, {90, 0, -90, 358}, 180, 91},
        {"014", Increments{2, 2}, {89, 1, -89, 359}, 180, 90},
        {"015", Increments{2, 2}, {89.7626, 1.1093, -88.2374, 359.1093}, 180, 90},
        {"016", Increments{1, 1}, {90, 0, -90, 359}, 360, 181},
        {"017", Increments{0.5, 0.5}, {90, 0, -90, 359.5}, 720, 361},
    };

    for (const auto& test : tests) {
        SECTION(test.name) {
            const PointLonLat reference{test.area.west(), test.area.south()};
            RepresentationHandle repres(new RegularLL(test.increments, test.area, reference));

            const auto& ll   = dynamic_cast<const RegularLL&>(*repres);
            const auto& bbox = repres->boundingBox();

            EXPECT_EQUAL(ll.Ni(), test.Ni);
            EXPECT_EQUAL(ll.Nj(), test.Nj);
            EXPECT_EQUAL(repres->numberOfPoints(), test.Ni * test.Nj);

            // the corrected area is within, from the reference point
            EXPECT(test.area.contains(bbox));
            EXPECT(eckit::types::is_approximately_equal(bbox.south(), reference.lat(), PointLonLat::EPS));
            EXPECT(eckit::types::is_approximately_equal(bbox.west(), reference.lon(), PointLonLat::EPS));
        }
    }
}


CASE("RegularLL: by parametrisation (as decoded), periodic within an increment (MIR-351)") {
    // GRIB2 encodes (micro-degrees) a West/East half an increment away from the meridian, not exactly
    const double inc = 45. / 128.;

    param::SimpleParametrisation param;
    param.set("gridType", "regular_ll");
    param.set("west_east_increment", inc);
    param.set("south_north_increment", inc);
    param.set("north", 45.);
    param.set("south", -45.);
    param.set("west", 0.175781);
    param.set("east", 359.824219);
    param.set("Ni", 1024L);
    param.set("Nj", 257L);

    RepresentationHandle repres(RepresentationFactory::build(param));

    EXPECT_EQUAL(repres->numberOfPoints(), 1024 * 257);
    EXPECT(eckit::types::is_approximately_equal(repres->boundingBox().west(), 0.175781));

    const auto domain = repres->domain();
    EXPECT_NOT(domain.isGlobal());
    EXPECT(domain.isPeriodicWestEast());
    EXPECT(repres->isPeriodicWestEast());
    EXPECT_NOT(repres->includesNorthPole());
    EXPECT_NOT(repres->includesSouthPole());

    // the given (Ni, Nj) must agree
    param.set("Ni", 1023L);
    EXPECT_THROWS_AS(RepresentationFactory::build(param), eckit::AssertionFailed);
}


CASE("RegularLL: fillGrib, by the order of the points") {
    struct test_t {
        std::string order;
        long iScansNegatively;
        long jScansPositively;
        double latitudeOfFirstGridPoint;
        double longitudeOfFirstGridPoint;
    };

    const std::vector<test_t> tests{
        {"i+j-", 0, 0, 10., -10.},
        {"i-j-", 1, 0, 10., 10.},
        {"i+j+", 0, 1, -10., -10.},
        {"i-j+", 1, 1, -10., 10.},
    };

    for (const auto& test : tests) {
        SECTION(test.order) {
            RepresentationHandle repres(make("{grid: [1, 1], area: [10, -10, -10, 10], order: " + test.order + "}"));
            EXPECT_EQUAL(repres->order(), test.order);

            grib_info info;
            repres->fillGrib(info);

            EXPECT_EQUAL(info.grid.grid_type, CODES_UTIL_GRID_SPEC_REGULAR_LL);
            EXPECT_EQUAL(info.grid.Ni, 21);
            EXPECT_EQUAL(info.grid.Nj, 21);
            EXPECT(same(info.grid.iDirectionIncrementInDegrees, 1.));
            EXPECT(same(info.grid.jDirectionIncrementInDegrees, 1.));
            EXPECT(same(info.grid.latitudeOfFirstGridPointInDegrees, test.latitudeOfFirstGridPoint));
            EXPECT(same(info.grid.longitudeOfFirstGridPointInDegrees, test.longitudeOfFirstGridPoint));
            EXPECT(same(info.grid.latitudeOfLastGridPointInDegrees, -test.latitudeOfFirstGridPoint));
            EXPECT(same(info.grid.longitudeOfLastGridPointInDegrees, -test.longitudeOfFirstGridPoint));
            EXPECT_EQUAL(info.grid.iScansNegatively, test.iScansNegatively);
            EXPECT_EQUAL(info.grid.jScansPositively, test.jScansPositively);
            EXPECT_EQUAL(extra_long(info, "expandBoundingBox"), 1);

            // values, to/from the canonical order (i+j-)
            MIRValuesVector values(21 * 21);
            for (size_t i = 0; i < values.size(); ++i) {
                values[i] = static_cast<double>(i);
            }

            auto copy = values;
            repres->reorderToCanonical(copy);
            EXPECT_EQUAL(copy.front(),
                         values[test.iScansNegatively != 0 ? 20 : 0] + (test.jScansPositively != 0 ? 21. * 20. : 0.));

            repres->reorderFromCanonical(copy);
            EXPECT(copy == values);
        }
    }
}


CASE("RegularLL: crop, extend, frame") {
    RepresentationHandle global(new RegularLL(Increments{1, 1}));
    EXPECT_EQUAL(global->numberOfPoints(), 360 * 181);
    EXPECT_EQUAL(global->factory(), "regular_ll");


    SECTION("croppedRepresentation") {
        RepresentationHandle cropped(global->croppedRepresentation({10.5, 20.5, -10.5, 30.5}));
        EXPECT_EQUAL(cropped->numberOfPoints(), 10 * 21);
        EXPECT(cropped->boundingBox() == BoundingBox(10, 21, -10, 30));
        EXPECT_NOT(cropped->isGlobal());
        EXPECT_NOT(cropped->sameAs(*global));
    }


    SECTION("extendBoundingBox (outwards, to the grid points)") {
        const BoundingBox bbox(10.5, 20.5, -10.5, 30.5);
        const auto extended = global->extendBoundingBox(bbox);

        EXPECT(extended.contains(bbox));
        EXPECT(extended == BoundingBox(11, 20, -11, 31));

        // not beyond the poles
        const auto polar = global->extendBoundingBox({89.5, 0, -89.5, 10});
        EXPECT(eckit::types::is_approximately_equal(polar.north(), NORTH_POLE, PointLonLat::EPS));
        EXPECT(eckit::types::is_approximately_equal(polar.south(), SOUTH_POLE, PointLonLat::EPS));

        // shifted grid, by the reference (South/West) point
        RepresentationHandle shifted(new RegularLL(Increments{1, 1}, {89.5, 0.5, -89.5, 359.5}, {0.5, -89.5}));
        const auto e = shifted->extendBoundingBox(bbox);
        EXPECT(e.contains(bbox));
        EXPECT(e == BoundingBox(10.5, 20.5, -10.5, 30.5));

        // shifted grid, without latitudes between the bounding box and the poles
        const BoundingBox towards(89.9, 0.5, -89.9, 10.2);
        const auto t = shifted->extendBoundingBox(towards);
        EXPECT(t.contains(towards));
        EXPECT(t == BoundingBox(89.9, 0.5, -89.9, 10.5));

        RepresentationHandle cropped(shifted->croppedRepresentation(t));
        EXPECT(cropped->boundingBox() == BoundingBox(89.5, 0.5, -89.5, 10.5));
        EXPECT_EQUAL(cropped->numberOfPoints(), 11 * 180);
    }


    SECTION("frame (missing values inside a frame)") {
        RepresentationHandle repres(new RegularLL(Increments{1, 1}, {2, 0, -2, 4}));
        MIRValuesVector values(5 * 5, 1.);

        EXPECT_EQUAL(repres->frame(values, 1, 9999.), 9);
        EXPECT_EQUAL(values[0], 1.);
        EXPECT_EQUAL(values[6], 9999.);
        EXPECT_EQUAL(values[12], 9999.);
        EXPECT_EQUAL(values[24], 1.);

        MIRValuesVector wrong(5 * 4);
        EXPECT_THROWS_AS(repres->frame(wrong, 1, 9999.), eckit::AssertionFailed);
    }


    SECTION("fillMeshGen (poles of regional grids)") {
        RepresentationHandle north(new RegularLL(Increments{1, 1}, {60, -10, 30, 40}));
        RepresentationHandle south(new RegularLL(Increments{1, 1}, {-30, -10, -60, 40}));

        util::MeshGeneratorParameters a;
        north->fillMeshGen(a);
        EXPECT_EQUAL(a.meshGenerator_, "structured");
        EXPECT(a.getBool("force_include_south_pole"));
        EXPECT_NOT(a.getBool("force_include_north_pole"));

        util::MeshGeneratorParameters b;
        b.meshGenerator_ = "delaunay";
        south->fillMeshGen(b);
        EXPECT_EQUAL(b.meshGenerator_, "delaunay");
        EXPECT(b.getBool("force_include_north_pole"));
        EXPECT_NOT(b.getBool("force_include_south_pole"));
    }
}


}  // namespace mir::repres::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
