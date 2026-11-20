// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <cmath>
#include <memory>
#include <string>
#include <vector>

#include "eckit/testing/Test.h"

#include "mir/param/SimpleParametrisation.h"
#include "mir/repres/UnstructuredGrid.h"
#include "mir/util/BoundingBox.h"
#include "mir/util/Domain.h"
#include "mir/util/MeshGeneratorParameters.h"

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


    SECTION("gridSpec: unstructured, by uid (the coordinates kept by eckit::geo)") {
        RepresentationHandle points(new UnstructuredGrid(LATITUDES, LONGITUDES));
        RepresentationHandle repres(make("{uid: " + points->grid().uid() + "}"));

        EXPECT_EQUAL(repres->numberOfPoints(), LATITUDES.size());
        EXPECT(repres->sameAs(*points));
    }
}


}  // namespace mir::repres::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
