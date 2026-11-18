// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <sstream>
#include <string>
#include <vector>

#include "eckit/testing/Test.h"

#include "mir/data/MIRField.h"
#include "mir/input/GribFileInput.h"
#include "mir/key/grid/Grid.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/util/MeshGeneratorParameters.h"
#include "mir/util/Rotation.h"

#include "repres.h"


namespace mir::repres::tests {


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


}  // namespace mir::repres::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
