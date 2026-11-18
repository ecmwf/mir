// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <memory>
#include <string>

#include "eckit/testing/Test.h"

#include "mir/repres/latlon/RegularLL.h"
#include "mir/repres/latlon/RotatedLL.h"
#include "mir/util/BoundingBox.h"
#include "mir/util/Domain.h"
#include "mir/util/Increments.h"
#include "mir/util/Rotation.h"

#include "repres.h"


namespace mir::repres::tests {


using latlon::LatLon;
using latlon::RegularLL;
using util::BoundingBox;
using util::Increments;


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
        EXPECT_NOT(same(it->pointRotated().lat(), 10.));
    }

    SECTION("cropped, and its geographic domain") {
        RepresentationHandle cropped(repres->croppedRepresentation({5, -5, -5, 5}));
        EXPECT_EQUAL(cropped->numberOfPoints(), 11 * 11);
        EXPECT_EQUAL(cropped->factory(), "rotated_ll");

        const auto geo = cropped->geographicDomain();
        EXPECT_NOT(geo == cropped->domain());
    }
}


}  // namespace mir::repres::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
