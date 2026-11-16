// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <cmath>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "eckit/filesystem/TmpFile.h"
#include "eckit/testing/Test.h"

#include "mir/api/mir_config.h"
#include "mir/data/MIRField.h"
#include "mir/input/GribFileInput.h"
#include "mir/key/grid/Grid.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/repres/UnstructuredGrid.h"
#include "mir/util/BoundingBox.h"
#include "mir/util/Domain.h"
#include "mir/util/MeshGeneratorParameters.h"
#include "mir/util/Rotation.h"

#include "repres.h"


namespace mir::repres::tests {


const std::vector<double> LATITUDES{60., 50., 40., 30., -10.};
const std::vector<double> LONGITUDES{-10., 0., 10., 20., 170.};


CASE("UnstructuredGrid") {
    RepresentationHandle repres(new UnstructuredGrid(LATITUDES, LONGITUDES));
    const auto n = LATITUDES.size();

    EXPECT_EQUAL(repres->numberOfPoints(), n);
    EXPECT_EQUAL(to_string(*repres), "UnstructuredGrid[points=5]");
    EXPECT(repres->uniqueName().find("unstructured-5-") == 0);

    EXPECT_NO_THROW(repres->validate(MIRValuesVector(n)));
    EXPECT_THROWS_AS(repres->validate(MIRValuesVector(n - 1)), eckit::AssertionFailed);


    SECTION("points, as given") {
        size_t i = 0;
        for (std::unique_ptr<Iterator> it(repres->iterator()); it->next(); ++i) {
            EXPECT_EQUAL(it->index(), i);
            EXPECT_EQUAL(it->pointUnrotated().lat().value(), LATITUDES[i]);
            EXPECT_EQUAL(it->pointUnrotated().lon().value(), LONGITUDES[i]);
        }
        EXPECT_EQUAL(i, n);

        EXPECT_EQUAL(repres->grid().size(), n);
        EXPECT_EQUAL(static_cast<size_t>(repres->atlasGrid().size()), n);
    }


    SECTION("comparisons") {
        RepresentationHandle same_points(new UnstructuredGrid(LATITUDES, LONGITUDES));
        RepresentationHandle other(new UnstructuredGrid({60., 50.}, {-10., 0.}));

        EXPECT(repres->sameAs(*same_points));
        EXPECT_NOT(repres->sameAs(*other));
        EXPECT_EQUAL(repres->uniqueName(), same_points->uniqueName());
        EXPECT(repres->uniqueName() != other->uniqueName());
    }


    SECTION("domain (the bounding box, global by default)") {
        EXPECT(repres->includesNorthPole());
        EXPECT(repres->includesSouthPole());
        EXPECT(repres->isPeriodicWestEast());
        EXPECT(repres->domain().isGlobal());

        RepresentationHandle regional(new UnstructuredGrid(LATITUDES, LONGITUDES, {70, -20, -20, 180}));
        EXPECT_NOT(regional->includesNorthPole());
        EXPECT_NOT(regional->includesSouthPole());
        EXPECT_NOT(regional->isPeriodicWestEast());
        EXPECT(regional->domain() == util::Domain(70, -20, -20, 180));
    }


    SECTION("fillGrib, fillMeshGen") {
        grib_info info;
        repres->fillGrib(info);
        EXPECT_EQUAL(info.grid.grid_type, CODES_UTIL_GRID_SPEC_UNSTRUCTURED);
        EXPECT_EQUAL(info.packing.editionNumber, 2);

        util::MeshGeneratorParameters mesh;
        repres->fillMeshGen(mesh);
        EXPECT_EQUAL(mesh.meshGenerator_, "delaunay");
    }


    SECTION("croppedRepresentation") {
        RepresentationHandle cropped(repres->croppedRepresentation({55, -15, 25, 15}));
        EXPECT_EQUAL(cropped->numberOfPoints(), 2);

        std::unique_ptr<Iterator> it(cropped->iterator());
        ASSERT(it->next());
        EXPECT_EQUAL(it->pointUnrotated().lat().value(), 50.);

        // nothing to crop: the same representation
        RepresentationHandle same_repres(repres->croppedRepresentation(util::BoundingBox{}));
        EXPECT(static_cast<const Representation*>(same_repres) == static_cast<const Representation*>(repres));
    }


    SECTION("errors") {
        EXPECT_THROWS_AS(UnstructuredGrid({1., 2.}, {1.}), exception::UserError);
        EXPECT_THROWS_AS(UnstructuredGrid({1., 2., 1.}, {1., 2., 1.}), exception::UserError);  // duplicate points
    }
}


CASE("UnstructuredGrid: by parametrisation, by gridSpec, by file") {
    SECTION("latitudes/longitudes") {
        param::SimpleParametrisation param;
        param.set("gridType", "unstructured_grid");
        param.set("latitudes", LATITUDES);
        param.set("longitudes", LONGITUDES);

        RepresentationHandle repres(RepresentationFactory::build(param));
        EXPECT_EQUAL(repres->numberOfPoints(), LATITUDES.size());
        EXPECT(repres->sameAs(*RepresentationHandle(new UnstructuredGrid(LATITUDES, LONGITUDES))));

        param.set("longitudes", std::vector<double>{1., 2.});
        EXPECT_THROWS_AS(RepresentationFactory::build(param), exception::UserError);
    }


    SECTION("numberOfPoints (unusable coordinates)") {
        param::SimpleParametrisation param;
        param.set("gridType", "unstructured_grid");
        EXPECT_THROWS_AS(RepresentationFactory::build(param), exception::UserError);

        param.set("numberOfPoints", 4L);
        RepresentationHandle repres(RepresentationFactory::build(param));
        EXPECT_EQUAL(repres->numberOfPoints(), 4);

        std::unique_ptr<Iterator> it(repres->iterator());
        ASSERT(it->next());
        EXPECT(std::isnan(it->pointUnrotated().lat().value()));
    }


    SECTION("gridSpec: unstructured, by latitudes/longitudes") {
        RepresentationHandle repres(make("{type: unstructured_ll, latitudes: [60, 50, 40], longitudes: [-10, 0, 10]}"));
        EXPECT_EQUAL(repres->numberOfPoints(), 3);
        EXPECT_EQUAL(count(*repres), 3);
    }


    SECTION("gridSpec: other grids, by their points") {
        RepresentationHandle repres(
            make("{type: irregular_ll, latitudes: [60, 50, 45, 30], longitudes: [-10, 0, 15]}"));
        EXPECT_EQUAL(repres->numberOfPoints(), 4 * 3);
        EXPECT(repres->uniqueName().find("unstructured-12-") == 0);
    }


    SECTION("file (griddef)") {
        eckit::TmpFile path;
        UnstructuredGrid::save(path, LATITUDES, LONGITUDES, true);

        RepresentationHandle repres(new UnstructuredGrid(path));
        EXPECT(repres->sameAs(*RepresentationHandle(new UnstructuredGrid(LATITUDES, LONGITUDES))));
    }
}


CASE("IrregularLatlon") {
    auto build = [](const std::vector<double>& latitudes, const std::vector<double>& longitudes) {
        param::SimpleParametrisation param;
        param.set("gridType", "irregular_latlon");
        param.set("latitudes", latitudes);
        param.set("longitudes", longitudes);
        return RepresentationFactory::build(param);
    };

    RepresentationHandle regional(build({60., 50., 45., 30.}, {-10., 0., 15.}));
    RepresentationHandle global(build({90., 60., 30., 0., -30., -60., -90.}, {0., 60., 120., 180., 240., 300.}));

    EXPECT_EQUAL(regional->numberOfPoints(), 4 * 3);
    EXPECT_EQUAL(global->numberOfPoints(), 7 * 6);

    EXPECT_EQUAL(to_string(*regional), "IrregularLatlon[latitudes=4,longitudes=3]");
    EXPECT(regional->uniqueName().find("irregular-latlon-4-3-") == 0);
    RepresentationHandle again(build({60., 50., 45., 30.}, {-10., 0., 15.}));
    EXPECT(regional->sameAs(*again));
    EXPECT_NOT(regional->sameAs(*global));

    EXPECT_NO_THROW(regional->validate(MIRValuesVector(12)));
    EXPECT_THROWS_AS(regional->validate(MIRValuesVector(13)), eckit::AssertionFailed);


    SECTION("domain") {
        EXPECT_NOT(regional->isPeriodicWestEast());
        EXPECT_NOT(regional->includesNorthPole());
        EXPECT_NOT(regional->includesSouthPole());
        EXPECT(regional->domain() == util::Domain(60, -10, 30, 15));

        EXPECT(global->isPeriodicWestEast());
        EXPECT(global->includesNorthPole());
        EXPECT(global->includesSouthPole());
        EXPECT(global->isGlobal());
    }


    SECTION("meshing, element diagonal") {
        util::MeshGeneratorParameters mesh;
        regional->fillMeshGen(mesh);
        EXPECT_EQUAL(mesh.meshGenerator_, "delaunay");

        double d = 0.;
        EXPECT(regional->getLongestElementDiagonal(d));
        EXPECT(d > 0.);

        EXPECT_EQUAL(static_cast<size_t>(regional->atlasGrid().size()), 12);
    }


    SECTION("not supported") {
        grib_info info;
        EXPECT_THROWS_AS(regional->fillGrib(info), eckit::NotImplemented);
    }
}


#if mir_HAVE_ECKIT_CODEC
CASE("ORCA, ICON and FESOM grids: names") {
    struct test_t {
        std::string name;
        std::string canonical;
    };

    const std::vector<test_t> tests{
        {"orca2", "ORCA2_T"},
        {"EORCA1", "eORCA1_T"},
        {"eorca025_v", "eORCA025_V"},
        {"eORCA1_T", "eORCA1_T"},
        {"ICON-CH2", "icon-ch2-v1"},
        {"icon-ch1-v2", "icon-ch1-v2"},
        {"icon-grid-0026-R03B07-G", "icon-grid-0026-r03b07-g"},
        {"core2", "CORE2_C"},
        {"pi_n", "pi_N"},
        {"NG5_C", "NG5_C"},
    };

    param::SimpleParametrisation param;
    for (const auto& test : tests) {
        EXPECT_EQUAL(key::grid::Grid::canonical(test.name, param), test.canonical);
    }
}


CASE("ORCA grid") {
    SECTION("by gridSpec") {
        RepresentationHandle repres(make("{grid: eORCA1_T}"));
        const auto n = repres->numberOfPoints();

        EXPECT_EQUAL(n, 120184);
        EXPECT_EQUAL(count(*repres), n);
        EXPECT(repres->uniqueName().find("ORCA-120184-") == 0);
        EXPECT_EQUAL(to_string(*repres), repres->spec().str());
        EXPECT(repres->sameAs(*make("{grid: eORCA1_T}")));
        EXPECT_NOT(repres->sameAs(*make("{grid: eORCA1_U}")));

        EXPECT_NO_THROW(repres->validate(MIRValuesVector(n)));
        EXPECT_THROWS_AS(repres->validate(MIRValuesVector(n + 1)), eckit::AssertionFailed);

        grib_info info;
        repres->fillGrib(info);
        EXPECT_EQUAL(info.grid.grid_type, CODES_UTIL_GRID_SPEC_UNSTRUCTURED);
        EXPECT_EQUAL(info.packing.editionNumber, 2);
        EXPECT_EQUAL(extra_string(info, "unstructuredGridType"), "eORCA1");
        EXPECT_EQUAL(extra_string(info, "unstructuredGridSubtype"), "T");
        EXPECT_EQUAL(extra_string(info, "uuidOfHGrid"), repres->grid().uid());

        util::MeshGeneratorParameters mesh;
        repres->fillMeshGen(mesh);
        EXPECT(!mesh.meshGenerator_.empty());

        EXPECT_EQUAL(static_cast<size_t>(repres->atlasGrid().size()), n);
    }


    SECTION("by name") {
        const auto& grid = key::grid::Grid::lookup("orca2");
        EXPECT_EQUAL(grid.gridname(), "ORCA2_T");

        std::ostringstream str;
        str << grid;
        EXPECT(str.str().find("ORCA2_T") != std::string::npos);
        EXPECT_EQUAL(grid.gaussianNumber(), key::grid::Grid::default_gaussian_number());
        EXPECT_THROWS_AS(grid.representation(util::Rotation(-40., 22.)), eckit::NotImplemented);

        RepresentationHandle repres(grid.representation());
        EXPECT_EQUAL(repres->numberOfPoints(), repres->grid().size());
        EXPECT(repres->uniqueName().find("ORCA-") == 0);
    }


    SECTION("by gridType and uid") {
        RepresentationHandle uid(make("{grid: eORCA1_T}"));

        param::SimpleParametrisation param;
        param.set("gridType", "orca");
        param.set("uid", uid->grid().uid());

        RepresentationHandle repres(RepresentationFactory::build(param));
        EXPECT(repres->sameAs(*uid));
    }


    SECTION("by GRIB (settings of the grid type, uid from the grid catalog)") {
        input::GribFileInput input("ORCA.grib2");
        ASSERT(input.next());

        const input::MIRInput& in = input;

        std::string intgrid;
        EXPECT(in.parametrisation().get("intgrid", intgrid));
        EXPECT_EQUAL(intgrid, "O96");

        std::string interpolation;
        EXPECT(in.parametrisation().get("intermediate-interpolation", interpolation));
        EXPECT_EQUAL(interpolation, "nn");

        RepresentationHandle repres(in.field().representation());
        EXPECT_EQUAL(repres->numberOfPoints(), 120184);
    }


    SECTION("other grid types have no such settings") {
        input::GribFileInput input("gridType=reduced_gg,gridName=O32,shortName=msl.grib1");
        ASSERT(input.next());

        const input::MIRInput& in = input;

        std::string value;
        EXPECT_NOT(in.parametrisation().get("intgrid", value));
        EXPECT_NOT(in.parametrisation().get("intermediate-interpolation", value));
    }
}


CASE("ICON grid") {
    RepresentationHandle repres(make("{grid: ICON-CH2}"));
    const auto n = repres->numberOfPoints();

    EXPECT_EQUAL(n, repres->grid().size());
    EXPECT(repres->uniqueName().find("ICON-") == 0);
    EXPECT_NO_THROW(repres->validate(MIRValuesVector(n)));
    EXPECT_THROWS_AS(repres->validate(MIRValuesVector(n + 1)), eckit::AssertionFailed);
    EXPECT_NOT(repres->isGlobal());

    grib_info info;
    repres->fillGrib(info);
    EXPECT_EQUAL(info.grid.grid_type, CODES_UTIL_GRID_SPEC_UNSTRUCTURED);
    EXPECT_EQUAL(info.packing.editionNumber, 2);
    EXPECT_EQUAL(extra_string(info, "uuidOfHGrid"), repres->grid().uid());

    const auto& grid = key::grid::Grid::lookup("ICON-CH2");
    EXPECT_EQUAL(grid.gridname(), "icon-ch2-v1");
    EXPECT_EQUAL(grid.gaussianNumber(), key::grid::Grid::default_gaussian_number());

    RepresentationHandle named(grid.representation());
    EXPECT(named->sameAs(*repres));

    std::ostringstream str;
    str << grid;
    EXPECT(str.str().find("icon-ch2-v1") != std::string::npos);
    EXPECT_THROWS_AS(grid.representation(util::Rotation(-40., 22.)), eckit::NotImplemented);
}
#endif


}  // namespace mir::repres::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
