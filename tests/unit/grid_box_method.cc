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

#include "mir/param/SimpleParametrisation.h"
#include "mir/repres/gauss/reduced/ReducedFromPL.h"
#include "mir/repres/gauss/regular/RegularGG.h"
#include "mir/repres/latlon/ReducedLL.h"
#include "mir/repres/latlon/RegularLL.h"
#include "mir/util/BoundingBox.h"
#include "mir/util/Domain.h"
#include "mir/util/GridBox.h"
#include "mir/util/Increments.h"
#include "mir/util/Types.h"

// define EXPECTV(a) log << "\tEXPECT(" << #a <<")" << std::endl; EXPECT(a)


namespace mir::tests::unit {


CASE("grid boxes: West-East periodicity") {
    using Longitude = LongitudeDouble;


    SECTION("reduced Gaussian grid (periodic)") {
        const std::vector<long> pl{20, 20};

        repres::RepresentationHandle r(new repres::gauss::reduced::ReducedFromPL(pl.size() / 2, pl));
        ASSERT(r->domain().isPeriodicWestEast());

        auto boxes = r->gridBoxes();
        auto a     = boxes.front();
        auto b     = boxes.back();
        auto inc   = 360. / static_cast<double>(pl[0]);

        EXPECT_EQUAL(Longitude(a.west()), Longitude(-inc / 2.));
        EXPECT_EQUAL(Longitude(b.east()), Longitude(-inc / 2. + 360.));
    }


    SECTION("reduced Gaussian grid (non-periodic)") {
        const std::vector<long> pl{20, 20};
        util::BoundingBox bbox{90., 0., 0., 180.};

        repres::RepresentationHandle r(new repres::gauss::reduced::ReducedFromPL(pl.size() / 2, pl, bbox));
        ASSERT(!r->domain().isPeriodicWestEast());

        for (const auto& box : r->gridBoxes()) {
            EXPECT(bbox.contains({box.north(), box.west(), box.south(), box.east()}));
        }
    }


    SECTION("regular Gaussian grid (periodic)") {
        repres::RepresentationHandle r(new repres::gauss::regular::RegularGG(1));
        ASSERT(r->domain().isPeriodicWestEast());

        auto boxes = r->gridBoxes();
        auto a     = boxes.front();
        auto b     = boxes.back();
        auto inc   = 360. / 4.;  // assumes Ni == 4 * N

        EXPECT_EQUAL(Longitude(a.west()), Longitude(-inc / 2.));
        EXPECT_EQUAL(Longitude(b.east()), Longitude(-inc / 2. + 360.));
    }


    SECTION("regular Gaussian grid (non-periodic)") {
        util::BoundingBox bbox{90., 0., 0., 180.};

        repres::RepresentationHandle r(new repres::gauss::regular::RegularGG(1, bbox));
        ASSERT(!r->domain().isPeriodicWestEast());

        for (const auto& box : r->gridBoxes()) {
            EXPECT(bbox.contains({box.north(), box.west(), box.south(), box.east()}));
        }
    }


    SECTION("regular lat/lon grid (periodic)") {
        double inc = 18.;

        repres::RepresentationHandle r(new repres::latlon::RegularLL(util::Increments{inc, 90.}));
        ASSERT(r->domain().isPeriodicWestEast());

        auto boxes = r->gridBoxes();
        auto a     = boxes.front();
        auto b     = boxes.back();

        EXPECT_EQUAL(Longitude(a.west()), Longitude(-inc / 2.));
        EXPECT_EQUAL(Longitude(b.east()), Longitude(-inc / 2. + 360.));
    }


    SECTION("regular lat/lon grid (non-periodic)") {
        util::BoundingBox bbox{90., 0., 0., 180.};
        double inc = 18.;

        repres::RepresentationHandle r(new repres::latlon::RegularLL(util::Increments{inc, 90.}, bbox));
        ASSERT(!r->domain().isPeriodicWestEast());

        for (const auto& box : r->gridBoxes()) {
            EXPECT(bbox.contains({box.north(), box.west(), box.south(), box.east()}));
        }
    }


    {
        const std::vector<long> pl{20, 20};

        param::SimpleParametrisation param;
        param.set("pl", pl);
        param.set("Nj", pl.size());
        param.set("north", 90.);
        param.set("west", 0.);
        param.set("south", 0.);
        param.set("east", 360.);


        SECTION("reduced lat/lon grid (periodic)") {
            repres::RepresentationHandle r(new repres::latlon::ReducedLL(param));
            ASSERT(r->domain().isPeriodicWestEast());

            auto boxes = r->gridBoxes();
            auto a     = boxes.front();
            auto b     = boxes.back();
            auto inc   = 360. / static_cast<double>(pl[0]);

            EXPECT_EQUAL(Longitude(a.west()), Longitude(-inc / 2.));
            EXPECT_EQUAL(Longitude(b.east()), Longitude(-inc / 2. + 360.));
        }


        SECTION("reduced lat/lon grid (non-periodic)") {
            util::BoundingBox bbox{90., 0., 0., 180.};
            param.set("east", bbox.east().value());

            repres::RepresentationHandle r(new repres::latlon::ReducedLL(param));
            ASSERT(!r->domain().isPeriodicWestEast());
            for (const auto& box : r->gridBoxes()) {
                EXPECT(bbox.contains({box.north(), box.west(), box.south(), box.east()}));
            }
        }
    }
}


}  // namespace mir::tests::unit


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
