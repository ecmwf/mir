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


#include <string>
#include "eckit/log/Log.h"
#include "eckit/testing/Test.h"
#include "mir/key/grid/NamedGrid.h"
#include "mir/repres/Representation.h"
#include "mir/util/BoundingBox.h"

// define EXPECTV(a) log << "\tEXPECT(" << #a <<")" << std::endl; EXPECT(a)


namespace mir {
namespace tests {
namespace unit {


CASE("NamedGrid") {

    using util::BoundingBox;

    auto& log = eckit::Log::info();
    auto old  = log.precision(16);


    struct test_t {
        const std::string gridname;
        size_t numberOfPoints;
        size_t numberOfCroppedPoints;
        BoundingBox bbox;
    };

    std::vector<test_t> tests{
        {"F128", 131072, 858, BoundingBox(80.351, -150.46875, 79.648, 150.46875)},
        {"F128", 131072, 429, BoundingBox(80.350, -150.46875, 79.648, 150.46875)},
        {"N128", 88838, 88838, BoundingBox(90, 0, -90, 359.297)},
        {"F320", 819200, 912, BoundingBox(56, -16.5, 50.5, -3)},
        {"N320", 542080, 13437, BoundingBox(-60, 50, -90, 180)},
        {"O320", 421120, 16133, BoundingBox(58, -130, 25, -60)},
        {"O320", 421120, 7511, BoundingBox(-25, 135, -47, 179)},
        {"O320", 421120, 28101, BoundingBox(12, -81, -50, -35)},
        {"O320", 421120, 9877, BoundingBox(44, 91, 21, 143)},
        {"O320", 421120, 8743, BoundingBox(60, 23, 35, 80)},
        {"O640", 1661440, 4512, BoundingBox(52, 7, 43, 27.7)},
        {"N640", 2140702, 52654, BoundingBox(-60, 50, -90, 180)},
        {"O640", 1661440, 63842, BoundingBox(58, -130, 25, -60)},
        {"O640", 1661440, 29674, BoundingBox(-25, 135, -47, 179)},
        {"F1280", 13107200, 910080, BoundingBox(75, -30, 25, 60)},
        {"O1280", 6599680, 442460, BoundingBox(12, -81, -50, -35)},
        {"O1280", 6599680, 12370, BoundingBox(37.6025, -114.8907, 27.7626, -105.1875)},
        {"O1280", 6599680, 6599646, BoundingBox(90, 0, -90, 359.929)},
        {"O1280", 6599680, 118210, BoundingBox(-25, 135, -47, 179)},
    };

    SECTION("numberOfPoints") {
        for (const auto& t : tests) {

            log << "Test " + t.gridname + " (global)" << std::endl;
            repres::RepresentationHandle global(namedgrids::NamedGrid::lookup(t.gridname).representation());
            EXPECT(global->numberOfPoints() == t.numberOfPoints);

            log << "Test " + t.gridname + " (cropped)" << std::endl;
            repres::RepresentationHandle local(global->croppedRepresentation(t.bbox));
            EXPECT(local->numberOfPoints() == t.numberOfCroppedPoints);
        }
    }

    log.precision(old);
}


}  // namespace unit
}  // namespace tests
}  // namespace mir


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
