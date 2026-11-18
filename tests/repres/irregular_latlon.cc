// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <string>
#include <vector>

#include "eckit/testing/Test.h"

#include "mir/param/SimpleParametrisation.h"
#include "mir/util/Domain.h"
#include "mir/util/MeshGeneratorParameters.h"

#include "repres.h"


namespace mir::repres::tests {


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


}  // namespace mir::repres::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
