// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <memory>
#include <string>
#include <vector>

#include "eckit/testing/Test.h"
#include "eckit/types/FloatCompare.h"

#include "mir/data/MIRField.h"
#include "mir/input/GribFileInput.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/repres/latlon/RegularLL.h"
#include "mir/repres/latlon/RotatedLL.h"
#include "mir/util/BoundingBox.h"
#include "mir/util/Domain.h"
#include "mir/util/GridBox.h"
#include "mir/util/Increments.h"
#include "mir/util/MeshGeneratorParameters.h"
#include "mir/util/Rotation.h"

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
            PointLatLon reference(test.area.south(), test.area.west());
            RepresentationHandle repres(new RegularLL(test.increments, test.area, reference));

            const auto& ll   = dynamic_cast<const RegularLL&>(*repres);
            const auto& bbox = repres->boundingBox();

            EXPECT_EQUAL(ll.Ni(), test.Ni);
            EXPECT_EQUAL(ll.Nj(), test.Nj);
            EXPECT_EQUAL(repres->numberOfPoints(), test.Ni * test.Nj);

            // the corrected area is within, from the reference point
            EXPECT(test.area.contains(bbox));
            EXPECT(bbox.south() == reference.lat());
            EXPECT(bbox.west() == reference.lon());
        }
    }
}


CASE("LatLon::globaliseBoundingBox") {
    struct test_t {
        std::string name;
        Increments increments;
        BoundingBox area;
        BoundingBox global;
        size_t Ni;
        size_t Nj;
    };

    const std::vector<test_t> tests{
        {"001", Increments{1, 1}, {0, -350, 0, 8}, {90, 0, -90, 359}, 360, 181},
        {"002", Increments{0.5, 0.5}, {0, -350, 0, 9}, {90, 0, -90, 359.5}, 720, 361},
        {"003", Increments{2, 2}, {2, 0, 0, 2}, {90, 0, -90, 358}, 180, 91},
        {"004", Increments{2, 2}, {2.1, 0, 0, 2.1}, {90, 0, -90, 358}, 180, 91},
        {"005", Increments{2, 2}, {3, 1, 1, 3}, {89, 1, -89, 359}, 180, 90},
        {"006",
         Increments{2, 2},
         {37.6025, -114.8907, 27.7626, -105.1875},
         {89.7626, 1.1093, -88.2374, 359.1093},
         180,
         90},
        {"007", Increments{2, 2}, {88, -178, -88, 180}, {90, 0, -90, 358}, 180, 91},
        {"008", Increments{7, 7}, {85, 0, -90, 357}, {85, 0, -90, 357}, 52, 26},  // not periodic
        {"009", Increments{7, 7}, BoundingBox{}, {85, 0, -90, 357}, 52, 26},
        {"010", Increments{4, 4}, BoundingBox{}, {90, 0, -90, 356}, 90, 46},
    };

    for (const auto& test : tests) {
        SECTION(test.name) {
            PointLatLon reference(test.area.south(), test.area.west());

            BoundingBox global(test.area);
            LatLon::globaliseBoundingBox(global, test.increments, reference);

            EXPECT(same(global.north().value(), test.global.north().value(), 1e-6));
            EXPECT(same(global.west().value(), test.global.west().value(), 1e-6));
            EXPECT(same(global.south().value(), test.global.south().value(), 1e-6));
            EXPECT(same(global.east().value(), test.global.east().value(), 1e-6));

            RepresentationHandle repres(new RegularLL(test.increments, global, reference));
            const auto& ll = dynamic_cast<const RegularLL&>(*repres);

            EXPECT_EQUAL(ll.Ni(), test.Ni);
            EXPECT_EQUAL(ll.Nj(), test.Nj);
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
    EXPECT(eckit::types::is_approximately_equal(repres->boundingBox().west().value(), 0.175781));

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


CASE("RotatedLL") {
    const util::Rotation rotation(-40., 22.);
    RepresentationHandle repres(new latlon::RotatedLL(Increments{1, 1}, rotation, {10, -10, -10, 10}));
    RepresentationHandle unrotated(new RegularLL(Increments{1, 1}, {10, -10, -10, 10}));

    EXPECT_EQUAL(repres->numberOfPoints(), 21 * 21);
    EXPECT_EQUAL(count(*repres), 21 * 21);
    EXPECT_EQUAL(repres->factory(), "rotated_ll");

    EXPECT_NOT(repres->sameAs(*unrotated));
    EXPECT_NOT(unrotated->sameAs(*repres));
    EXPECT(repres->uniqueName() != unrotated->uniqueName());
    EXPECT(to_string(*repres).find("rotation") != std::string::npos);

    SECTION("fillGrib") {
        grib_info info;
        repres->fillGrib(info);

        EXPECT_EQUAL(info.grid.grid_type, CODES_UTIL_GRID_SPEC_ROTATED_LL);
        EXPECT_EQUAL(info.grid.Ni, 21);
        EXPECT_EQUAL(info.grid.Nj, 21);
        EXPECT(same(info.grid.latitudeOfSouthernPoleInDegrees, -40.));
        EXPECT(same(info.grid.longitudeOfSouthernPoleInDegrees, 22.));
        EXPECT(same(info.grid.angleOfRotationInDegrees, 0.));
    }

    SECTION("points, in the geographic frame") {
        std::unique_ptr<Iterator> it(repres->iterator());
        ASSERT(it->next());

        // the first point of the grid frame (north-west), rotated
        const auto& p = it->pointUnrotated();
        EXPECT(same(p.lat().value(), 10.));
        EXPECT(same(p.lon().value(), -10.));
        EXPECT_NOT(same((*(*it))[0], 10.));
    }

    SECTION("cropped, and its geographic domain") {
        RepresentationHandle cropped(repres->croppedRepresentation({5, -5, -5, 5}));
        EXPECT_EQUAL(cropped->numberOfPoints(), 11 * 11);
        EXPECT_EQUAL(cropped->factory(), "rotated_ll");

        const auto geo = cropped->geographicDomain();
        EXPECT_NOT(geo == cropped->domain());
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
        EXPECT(polar.north() == Latitude::NORTH_POLE);
        EXPECT(polar.south() == Latitude::SOUTH_POLE);

        // shifted grid
        RepresentationHandle shifted(new RegularLL(Increments{1, 1}, {89.5, 0.5, -89.5, 359.5}, {0.5, 0.5}));
        const auto e = shifted->extendBoundingBox(bbox);
        EXPECT(e.contains(bbox));
        EXPECT(e == BoundingBox(10.5, 20.5, -10.5, 30.5));
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


CASE("LatLon::globalise (towards the poles)") {
    RepresentationHandle repres(new RegularLL(Increments{10, 10}, {40, 0, -40, 350}));
    EXPECT_EQUAL(repres->numberOfPoints(), 36 * 9);

    MIRValuesVector values(36 * 9);
    for (size_t i = 0; i < values.size(); ++i) {
        values[i] = static_cast<double>(i);
    }

    data::MIRField field(repres, false, 9999.);
    field.update(values, 0);

    RepresentationHandle global(repres->globalise(field));
    ASSERT(global);

    EXPECT(global->isGlobal());
    EXPECT_EQUAL(global->numberOfPoints(), 36 * 19);

    // rows north of 40 are missing values (5 rows of 36), followed by the values
    const auto& v = field.values(0);
    EXPECT_EQUAL(v.size(), 36 * 19);
    EXPECT(field.hasMissing());
    EXPECT_EQUAL(v[0], 9999.);
    EXPECT_EQUAL(v[5 * 36 - 1], 9999.);
    EXPECT_EQUAL(v[5 * 36], 0.);
    EXPECT_EQUAL(v[5 * 36 + 36 * 9 - 1], 36. * 9. - 1.);
    EXPECT_EQUAL(v[5 * 36 + 36 * 9], 9999.);

    // already global
    data::MIRField again(global, true, 9999.);
    again.update(field.direct(0), 0);
    EXPECT(global->globalise(again) == nullptr);
}


CASE("ReducedLL") {
    const std::vector<long> pl{4, 8, 12, 8, 4};

    auto set = [&pl](param::SimpleParametrisation& p, double east) {
        p.set("gridType", "reduced_ll");
        p.set("pl", pl);
        p.set("Nj", 5L);
        p.set("north", 90.);
        p.set("west", 0.);
        p.set("south", -90.);
        p.set("east", east);
    };

    param::SimpleParametrisation param_global;
    set(param_global, 360.);

    param::SimpleParametrisation param_regional;
    set(param_regional, 180.);

    RepresentationHandle global(RepresentationFactory::build(param_global));
    RepresentationHandle regional(RepresentationFactory::build(param_regional));

    EXPECT_EQUAL(global->numberOfPoints(), 36);
    EXPECT(global->isPeriodicWestEast());
    EXPECT(global->includesNorthPole());
    EXPECT(global->includesSouthPole());
    EXPECT(global->isGlobal());

    EXPECT_EQUAL(regional->numberOfPoints(), 36);
    EXPECT_NOT(regional->isPeriodicWestEast());
    EXPECT_NOT(regional->isGlobal());

    EXPECT_NOT(global->sameAs(*regional));
    EXPECT(global->uniqueName() != regional->uniqueName());
    EXPECT(global->uniqueName().find("RLL5-") == 0);

    SECTION("element diagonal, grid boxes (periodic or not)") {
        double d1 = 0.;
        double d2 = 0.;
        EXPECT(global->getLongestElementDiagonal(d1));
        EXPECT(regional->getLongestElementDiagonal(d2));
        EXPECT(d1 > 0.);
        EXPECT(d2 > 0.);

        const auto boxes = regional->gridBoxes();
        EXPECT_EQUAL(boxes.size(), 36);
        for (const auto& box : boxes) {
            EXPECT(0. <= box.west() && box.east() <= 180.);
        }
    }

    SECTION("meshing (poles of regional grids)") {
        util::MeshGeneratorParameters mesh;
        regional->fillMeshGen(mesh);
        EXPECT_EQUAL(mesh.meshGenerator_, "structured");
        EXPECT_NOT(mesh.getBool("force_include_south_pole"));

        param::SimpleParametrisation p;
        set(p, 360.);
        p.set("south", 20.);
        RepresentationHandle north(RepresentationFactory::build(p));

        util::MeshGeneratorParameters a;
        north->fillMeshGen(a);
        EXPECT(a.getBool("force_include_south_pole"));
        EXPECT_NOT(a.getBool("force_include_north_pole"));

        param::SimpleParametrisation q;
        set(q, 360.);
        q.set("north", -20.);
        RepresentationHandle south(RepresentationFactory::build(q));

        util::MeshGeneratorParameters b;
        south->fillMeshGen(b);
        EXPECT(b.getBool("force_include_north_pole"));
        EXPECT_NOT(b.getBool("force_include_south_pole"));
    }

    SECTION("not supported") {
        grib_info info;
        EXPECT_THROWS_AS(global->fillGrib(info), eckit::NotImplemented);
        EXPECT_THROWS_AS(global->croppedRepresentation({10, 0, -10, 10}), exception::FunctionalityNotSupported);
    }

    SECTION("pl inconsistent with Nj") {
        param::SimpleParametrisation p;
        set(p, 360.);
        p.set("Nj", 4L);
        EXPECT_THROWS_AS(RepresentationFactory::build(p), eckit::AssertionFailed);
    }
}


CASE("ReducedLL (GRIB): periodic West-East (PGEN-492)") {
    const std::vector<std::string> files{"stream=wave,param=swh", "stream=wave,param=swh,domain=m"};

    for (const auto& file : files) {
        SECTION(file) {
            input::GribFileInput input(file);
            ASSERT(input.next());

            const input::MIRInput& in = input;

            std::string gridType;
            EXPECT(in.parametrisation().get("gridType", gridType));
            EXPECT_EQUAL(gridType, "reduced_ll");

            RepresentationHandle repres(in.field().representation());
            EXPECT(repres->domain().isPeriodicWestEast());
        }
    }
}


}  // namespace mir::repres::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
