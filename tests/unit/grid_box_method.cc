/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "eckit/testing/Test.h"

#include "eckit/types/FloatCompare.h"

#include "mir/param/SimpleParametrisation.h"
#include "mir/repres/gauss/reduced/ReducedFromPL.h"
#include "mir/repres/gauss/regular/RegularGG.h"
#include "mir/repres/latlon/ReducedLL.h"
#include "mir/repres/latlon/RegularLL.h"
#include "mir/util/BoundingBox.h"
#include "mir/util/Domain.h"
#include "mir/util/Exceptions.h"
#include "mir/util/GridBox.h"
#include "mir/util/Increments.h"
#include "mir/util/Log.h"
#include "mir/util/Types.h"


constexpr double EPS = 1e-6;

#define EXPECT_APPROX(a, b)                                             \
    Log::info() << "EXPECT(" << #a << " ~= " << #b << ")" << std::endl; \
    EXPECT(eckit::types::is_approximately_equal(static_cast<double>(a), static_cast<double>(b), EPS))

#define EXPECT_BOX(box, n, w, s, e) \
    EXPECT_APPROX(box.north(), n);  \
    EXPECT_APPROX(box.west(), w);   \
    EXPECT_APPROX(box.south(), s);  \
    EXPECT_APPROX(box.east(), e)


namespace mir::tests::unit {


CASE("grid boxes: West-East periodicity, reduced Gaussian grids") {
    using Longitude = LongitudeDouble;


    SECTION("periodic") {
        const std::vector<long> pl{20, 20};

        repres::RepresentationHandle r(new repres::gauss::reduced::ReducedFromPL(pl.size() / 2, pl));
        ASSERT(r->domain().isPeriodicWestEast());

        const auto boxes = r->gridBoxes(false);
        EXPECT(boxes.size() == r->numberOfPoints());
        EXPECT(boxes.size() == 40);

        const auto inc = 360. / static_cast<double>(pl[0]);

        EXPECT_EQUAL(Longitude(boxes.front().west()), Longitude(-inc / 2.));
        EXPECT_EQUAL(Longitude(boxes.back().east()), Longitude(-inc / 2. + 360.));

        EXPECT_BOX(boxes[0], 90., -9., 0., 9.);
        EXPECT_BOX(boxes[1], 90., 9., 0., 27.);
        // ...
        EXPECT_BOX(boxes[38], 0., 315., -90., 333.);
        EXPECT_BOX(boxes[39], 0., 333., -90., 351.);
    }


    SECTION("non-periodic") {
        const std::vector<long> pl{20, 20};
        util::BoundingBox bbox{90., 0., 0., 180.};

        repres::RepresentationHandle r(new repres::gauss::reduced::ReducedFromPL(pl.size() / 2, pl, bbox));
        ASSERT(!r->domain().isPeriodicWestEast());

        const auto boxes = r->gridBoxes(false);
        EXPECT(boxes.size() == r->numberOfPoints());
        EXPECT(boxes.size() == 11);

        for (const auto& box : boxes) {
            EXPECT(bbox.contains({box.north(), box.west(), box.south(), box.east()}));
        }

        EXPECT_BOX(boxes[0], 90., 0., 0., 9.);
        EXPECT_BOX(boxes[1], 90., 9., 0., 27.);
        // ...
        EXPECT_BOX(boxes[9], 90., 153., 0., 171.);
        EXPECT_BOX(boxes[10], 90., 171., 0., 180.);
    }
}


CASE("grid boxes: West-East periodicity, regular Gaussian grids") {

    SECTION("periodic") {
        repres::RepresentationHandle r(new repres::gauss::regular::RegularGG(1));
        ASSERT(r->domain().isPeriodicWestEast());

        const auto boxes = r->gridBoxes(false);
        EXPECT(boxes.size() == r->numberOfPoints());
        EXPECT(boxes.size() == 8);

        const auto inc = 360. / 4.;  // assumes Ni == 4 * N

        EXPECT_EQUAL(Longitude(boxes.front().west()), Longitude(-inc / 2.));
        EXPECT_EQUAL(Longitude(boxes.back().east()), Longitude(-inc / 2. + 360.));

        EXPECT_BOX(boxes[0], 90., -45., 0., 45.);
        EXPECT_BOX(boxes[1], 90., 45., 0., 135.);
        // ...
        EXPECT_BOX(boxes[6], 0., 135., -90., 225.);
        EXPECT_BOX(boxes[7], 0., 225., -90., 315.);
    }


    SECTION("non-periodic") {
        util::BoundingBox bbox{90., 0., 0., 180.};

        repres::RepresentationHandle r(new repres::gauss::regular::RegularGG(1, bbox));
        ASSERT(!r->domain().isPeriodicWestEast());

        const auto boxes = r->gridBoxes(false);
        EXPECT(boxes.size() == r->numberOfPoints());
        EXPECT(boxes.size() == 3);

        for (const auto& box : boxes) {
            EXPECT(bbox.contains({box.north(), box.west(), box.south(), box.east()}));
        }

        EXPECT_BOX(boxes[0], 90., 0., 35.26439, 45.);
        EXPECT_BOX(boxes[1], 90., 45., 35.26439, 135.);
        EXPECT_BOX(boxes[2], 90., 135., 35.26439, 180.);
    }
}


CASE("grid boxes: West-East periodicity, regular lat/lon grids") {
    SECTION("periodic") {
        double inc = 18.;

        repres::RepresentationHandle r(new repres::latlon::RegularLL(util::Increments{inc, 90.}));
        ASSERT(r->domain().isPeriodicWestEast());

        const auto boxes = r->gridBoxes(false);
        EXPECT(boxes.size() == r->numberOfPoints());
        EXPECT(boxes.size() == 60);

        EXPECT_EQUAL(Longitude(boxes.front().west()), Longitude(-inc / 2.));
        EXPECT_EQUAL(Longitude(boxes.back().east()), Longitude(-inc / 2. + 360.));

        EXPECT_BOX(boxes[0], 90., -9., 45., 9.);
        EXPECT_BOX(boxes[1], 90., 9., 45., 27.);
        // ...
        EXPECT_BOX(boxes[58], -45., 315., -90., 333.);
        EXPECT_BOX(boxes[59], -45., 333., -90., 351.);
    }


    SECTION("non-periodic") {
        util::BoundingBox bbox{90., 0., 0., 180.};
        double inc = 18.;

        repres::RepresentationHandle r(new repres::latlon::RegularLL(util::Increments{inc, 90.}, bbox));
        ASSERT(!r->domain().isPeriodicWestEast());

        const auto boxes = r->gridBoxes(false);
        EXPECT(boxes.size() == r->numberOfPoints());
        EXPECT(boxes.size() == 22);

        for (const auto& box : boxes) {
            EXPECT(bbox.contains({box.north(), box.west(), box.south(), box.east()}));
        }

        EXPECT_BOX(boxes[0], 90., 0., 45., 9.);
        EXPECT_BOX(boxes[1], 90., 9., 45., 27.);
        // ...
        EXPECT_BOX(boxes[20], 45., 153., 0., 171.);
        EXPECT_BOX(boxes[21], 45., 171., 0., 180.);
    }
}


CASE("grid boxes: West-East periodicity, reduced lat/lon grids") {
    SECTION("periodic") {
        const std::vector<long> pl{20, 20};

        param::SimpleParametrisation param;
        param.set("pl", pl);
        param.set("Nj", pl.size());
        param.set("north", 90.);
        param.set("west", 0.);
        param.set("south", 0.);
        param.set("east", 360.);

        repres::RepresentationHandle r(new repres::latlon::ReducedLL(param));
        ASSERT(r->domain().isPeriodicWestEast());

        const auto boxes = r->gridBoxes(false);
        EXPECT(boxes.size() == r->numberOfPoints());
        EXPECT(boxes.size() == 40);

        const auto inc = 360. / static_cast<double>(pl[0]);

        EXPECT_EQUAL(Longitude(boxes.front().west()), Longitude(-inc / 2.));
        EXPECT_EQUAL(Longitude(boxes.back().east()), Longitude(-inc / 2. + 360.));

        EXPECT_BOX(boxes[0], 90., -9., 45., 9.);
        EXPECT_BOX(boxes[1], 90., 9., 45., 27.);
        // ...
        EXPECT_BOX(boxes[38], 45., 315., 0., 333.);
        EXPECT_BOX(boxes[39], 45., 333., 0., 351.);
    }


    SECTION("non-periodic") {
        const std::vector<long> pl{11, 11};
        const util::BoundingBox bbox{90., 0., 0., 180.};

        param::SimpleParametrisation param;
        param.set("pl", pl);
        param.set("Nj", pl.size());
        param.set("north", bbox.north().value());
        param.set("west", bbox.west().value());
        param.set("south", bbox.south().value());
        param.set("east", bbox.east().value());

        repres::RepresentationHandle r(new repres::latlon::ReducedLL(param));
        ASSERT(!r->domain().isPeriodicWestEast());

        const auto boxes = r->gridBoxes(false);
        EXPECT(boxes.size() == r->numberOfPoints());
        EXPECT(boxes.size() == 22);

        for (const auto& box : boxes) {
            EXPECT(bbox.contains({box.north(), box.west(), box.south(), box.east()}));
        }

        EXPECT_BOX(boxes[0], 90., 0., 45., 9.);
        EXPECT_BOX(boxes[1], 90., 9., 45., 27.);
        // ...
        EXPECT_BOX(boxes[20], 45., 153., 0., 171.);
        EXPECT_BOX(boxes[21], 45., 171., 0., 180.);
    }
}


CASE("grid boxes dual: reduced Gaussian grids") {
    SECTION("periodic") {}
}


CASE("MIR-666") {
    // shifted grid, with finely-adjusted bounding box
    repres::RepresentationHandle r(new repres::latlon::RegularLL(
        util::Increments{0.05, 0.05}, util::BoundingBox{0.025, 27.875, -0.025, 57.825}, {-0.025, 27.875}));
    const auto boxes = r->gridBoxes(false);
}


}  // namespace mir::tests::unit


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
