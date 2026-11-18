// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <string>
#include <vector>

#include "eckit/testing/Test.h"

#include "mir/data/MIRField.h"
#include "mir/input/GribFileInput.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/util/Domain.h"
#include "mir/util/GridBox.h"
#include "mir/util/MeshGeneratorParameters.h"

#include "repres.h"


namespace mir::repres::tests {


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

    SECTION("factory, json") {
        EXPECT_EQUAL(global->factory(), "reduced_ll");
        EXPECT(to_json(*global).rfind(R"({"type":"reduced_ll","pl":[4,8,12,8,4],"bbox":)", 0) == 0);
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
