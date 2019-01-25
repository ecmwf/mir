/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include <iostream>
#include <string>
#include <vector>

#include "eckit/geometry/Point2.h"
#include "eckit/log/Log.h"
#include "eckit/testing/Test.h"
#include "eckit/memory/ScopedPtr.h"

#include "mir/api/Atlas.h"
#include "mir/repres/latlon/RotatedLL.h"
#include "mir/util/BoundingBox.h"
#include "mir/util/Increments.h"
#include "mir/util/Rotation.h"


namespace mir {
namespace tests {
namespace unit {


using util::BoundingBox;
using util::Increments;
using util::Rotation;


struct test_poles_t {
public:

    test_poles_t(const Increments& increments,
           const Rotation& rotation,
           const BoundingBox& bbox,
           bool includesNorthPole = false,
           bool includesSouthPole = false) :
        increments_(increments),
        rotation_(rotation),
        bbox_(bbox),
        includesNorthPole_(includesNorthPole),
        includesSouthPole_(includesSouthPole) {
    }

    const Increments increments_;
    const Rotation rotation_;
    const BoundingBox bbox_;
    const bool includesNorthPole_;
    const bool includesSouthPole_;

private:
    friend std::ostream& operator<<(std::ostream& out, const test_poles_t& test) {
        return (out << "test:"
                    << "\n\t" << "Increments  = " << test.increments_
                    << "\n\t" << "Rotation    = " << test.rotation_
                    << "\n\t" << "BoundingBox = " << test.bbox_
                    << "\n\t" << "includesNorthPole? " << test.includesNorthPole_
                    << "\n\t" << "includesSouthPole? " << test.includesSouthPole_);
    }
};


struct test_expected_rotation_t {
    const Increments increments_;
    const Rotation rotation_;
    const BoundingBox bbox_;
    const BoundingBox rotatedBbox_;

private:
    friend std::ostream& operator<<(std::ostream& out, const test_expected_rotation_t& test) {
        return (out << "test:"
                    << "\n\t" << "Increments  = " << test.increments_
                    << "\n\t" << "Rotation    = " << test.rotation_
                    << "\n\t" << "BoundingBox = " << test.bbox_
                    << "\n\t" << "rotatedBbox = " << test.rotatedBbox_);
    }
};


CASE("MIR-282") {
    auto& log = eckit::Log::info();
    auto old = log.precision(16);


    std::vector<test_poles_t> test_poles {
        { Increments(0.25, 0.25),   Rotation(-35.,   0.),  BoundingBox(12, -14.5, -17.25,  16.25) },
        { Increments(1., 1.),       Rotation(-90.,   0.),  BoundingBox(), true, true },
        { Increments(1., 1.),       Rotation(-75.,  15.),  BoundingBox(75., -35.,  20.,    45.), true },
        { Increments(1., 1.),       Rotation(-35.,  15.),  BoundingBox(40., -55., -45.,    55.), true },
        { Increments(1., 1.),       Rotation(-30., -15.),  BoundingBox(35., -40., -40.,    50.), true },
        { Increments(1., 1.),       Rotation(-25.,   0.),  BoundingBox(40., -50., -40.,    50.), true },
        { Increments(1., 1.),       Rotation(-15.,  45.),  BoundingBox(30., -50., -30.,     5.), true },
        { Increments(1., 1.),       Rotation(  0.,  80.),  BoundingBox(50., -65., -40.,    30.), true },

        { Increments(0.2,   0.2  ), Rotation(-40.,  10.),  BoundingBox(22.7,   -13.6,  -5.9, 21.8) },
        { Increments(0.1,   0.1  ), Rotation(-43.,  10.),  BoundingBox( 3.4,    -6.8,  -4.4,  4.8) },
        { Increments(0.1,   0.1  ), Rotation(-30.,   0.),  BoundingBox(18.1,   -37.6, -31.,  39.9) },
        { Increments(0.5,   0.5  ), Rotation(-76.,  14.),  BoundingBox(72.,    -32.,   20.,  42.) },
        { Increments(0.125, 0.125), Rotation(-30.,  -5.),  BoundingBox( 9.875, -15.,  -20.,  14.875) },

        { Increments(0.15,  0.15 ), Rotation(-15.,  45.),  BoundingBox(27.5,  -46.5,  -28,       1.5),  true },
        { Increments(0.3,   0.3  ), Rotation(  0., 130.),  BoundingBox(32.75, -86.75, -37.75,  -26.15), false },
        { Increments(0.25,  0.25 ), Rotation(-35.,  15.),  BoundingBox(36.,   -51.,   -41.,     51.),   true },

        { Increments(1., 1.),       Rotation(-35.,  160.), BoundingBox(80.,   30.,  75.,  200.) },
        { Increments(1., 1.),       Rotation( 30.,  -30.), BoundingBox(70.,  120.,  60.,  200.) },
        { Increments(1., 1.),       Rotation( 45., -120.), BoundingBox(55., -120., -10.,  140.) },

        { Increments(10., 10.),     Rotation(50.,  100.),  BoundingBox(10.,   70.,  -20.,  100.) },
    };


    std::vector<test_expected_rotation_t> test_expected_rotations {
        { Increments(1., 1.), Rotation(-40., 22.), BoundingBox(  90, -10,  70,  10), BoundingBox( 60, -164.730,  40, -151.270) },
        { Increments(1., 1.), Rotation(-40., 22.), BoundingBox( -70, -10, -90,  10), BoundingBox(-20,   18.370, -40,   25.630) },
    };


    SECTION("MIR-282: rotated_ll covering North/South poles") {
        for (auto& test : test_poles) {
            log << test << std::endl;

            const atlas::PointLonLat southPole(
                        test.rotation_.south_pole_longitude().normalise(Longitude::GREENWICH).value(),
                        test.rotation_.south_pole_latitude().value() );

            const atlas::util::Rotation r(southPole);

            // check bbox including poles (in the unrotated frame)
            atlas::PointLonLat NP{ r.unrotate({0., Latitude::NORTH_POLE.value()}) };
            atlas::PointLonLat SP{ r.unrotate({0., Latitude::SOUTH_POLE.value()}) };

            bool includesNorthPole = test.bbox_.contains(NP.lat(), NP.lon());
            bool includesSouthPole = test.bbox_.contains(SP.lat(), SP.lon());

            log << "check:"
                << "\n\t" << "includesNorthPole? " << includesNorthPole
                << "\n\t" << "includesSouthPole? " << includesSouthPole
                << std::endl;

            EXPECT(includesNorthPole == test.includesNorthPole_);
            EXPECT(includesSouthPole == test.includesSouthPole_);

        }
    }


    SECTION("MIR-282: rotated_ll contained by cropping") {
        for (auto& test : test_poles) {
            log << test << std::endl;

            repres::RepresentationHandle repres(new repres::latlon::RotatedLL(
                                                    test.increments_,
                                                    test.rotation_,
                                                    test.bbox_) );

            BoundingBox crop(test.rotation_.rotate(test.bbox_));

            log << "contained by cropping"
                << "\n\t" "   " << test.bbox_
                << "\n\t" " + " << test.rotation_
                << "\n\t" " = " << crop
                << std::endl;

            bool contains = true;
            for (eckit::ScopedPtr<repres::Iterator> iter(repres->iterator()); iter->next();) {
                auto& pr = iter->pointRotated();
                if (!crop.contains(pr[0], pr[1])) {
                    log << "!crop.contains:"
                        << "\t" << iter->pointRotated()
                        << "\t" << iter->pointUnrotated()
                        << std::endl;
                    contains = false;
                }
            }
            EXPECT(contains);
        }
    }


    SECTION("MIR-309: expected rotations") {

        auto is_approximately_equal = [] (const BoundingBox& a, const BoundingBox& b, double eps) {
            auto slightly_bigger = [eps](const BoundingBox& bbox) {

                Latitude n = std::min(Latitude::NORTH_POLE.value(), bbox.north().value() + eps);
                Latitude s = std::max(Latitude::SOUTH_POLE.value(), bbox.south().value() - eps);

                // periodicity not considered
                Longitude w = bbox.west().value() - eps;
                Longitude e = bbox.east().value() + eps;

                return BoundingBox(n, w, s, e);
            };

            return  slightly_bigger(a).contains(b) &&
                    slightly_bigger(b).contains(a);
        };


        for (auto& test : test_expected_rotations) {
            auto rotated = test.rotation_.rotate(test.bbox_);
            log << test << "\n\t" << test.rotatedBbox_ << " <- calculated" << std::endl;

            static constexpr double eps = 0.001;
            EXPECT(is_approximately_equal(test.rotatedBbox_, rotated, eps));
        }
    }


    log.precision(old);
}


}  // namespace unit
}  // namespace tests
}  // namespace mir


int main(int argc, char **argv) {
    return eckit::testing::run_tests(argc, argv, false);
}

