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

#include "eckit/testing/Test.h"

#include "mir/api/mir_config.h"
#include "mir/key/grid/Grid.h"
#include "mir/repres/Representation.h"
#include "mir/util/BoundingBox.h"
#include "mir/util/Log.h"

// define EXPECTV(a) log << "\tEXPECT(" << #a <<")" << std::endl; EXPECT(a)


namespace mir {
namespace tests {
namespace unit {


CASE("NamedGrid") {

    auto& log = Log::info();
    auto old  = log.precision(16);

    struct test_t {
        const std::string grid;
        size_t numberOfPoints;
        size_t numberOfCroppedPoints;
        util::BoundingBox bbox;
    };

    SECTION("numberOfPoints (F)") {
        std::vector<test_t> tests{
            {"F128", 131072, 858, {80.351, -150.46875, 79.648, 150.46875}},
            {"F128", 131072, 429, {80.350, -150.46875, 79.648, 150.46875}},
            {"F320", 819200, 912, {56, -16.5, 50.5, -3}},
            {"F1280", 13107200, 910080, {75, -30, 25, 60}},
        };

        for (const auto& t : tests) {
            log << "Test " + t.grid + " (global)" << std::endl;
            repres::RepresentationHandle global(key::grid::Grid::lookup(t.grid).representation());
            EXPECT(global->numberOfPoints() == t.numberOfPoints);

            log << "Test " + t.grid + " (cropped)" << std::endl;
            repres::RepresentationHandle local(global->croppedRepresentation(t.bbox));
            EXPECT(local->numberOfPoints() == t.numberOfCroppedPoints);
        }
    }

    SECTION("numberOfPoints (O)") {
        std::vector<test_t> tests{
            {"O320", 421120, 16133, {58, -130, 25, -60}},
            {"O320", 421120, 7511, {-25, 135, -47, 179}},
            {"O320", 421120, 28101, {12, -81, -50, -35}},
            {"O320", 421120, 9877, {44, 91, 21, 143}},
            {"O320", 421120, 8743, {60, 23, 35, 80}},
            {"O640", 1661440, 4512, {52, 7, 43, 27.7}},
            {"O640", 1661440, 63842, {58, -130, 25, -60}},
            {"O640", 1661440, 29674, {-25, 135, -47, 179}},
            {"O1280", 6599680, 442460, {12, -81, -50, -35}},
            {"O1280", 6599680, 12370, {37.6025, -114.8907, 27.7626, -105.1875}},
            {"O1280", 6599680, 6599646, {90, 0, -90, 359.929}},
            {"O1280", 6599680, 118210, {-25, 135, -47, 179}},
        };

        for (const auto& t : tests) {
            log << "Test " + t.grid + " (global)" << std::endl;
            repres::RepresentationHandle global(key::grid::Grid::lookup(t.grid).representation());
            EXPECT(global->numberOfPoints() == t.numberOfPoints);

            log << "Test " + t.grid + " (cropped)" << std::endl;
            repres::RepresentationHandle local(global->croppedRepresentation(t.bbox));
            EXPECT(local->numberOfPoints() == t.numberOfCroppedPoints);
        }
    }

#if mir_HAVE_ATLAS
    SECTION("numberOfPoints (N)") {
        std::vector<test_t> tests{
            {"N128", 88838, 88838, {90, 0, -90, 359.297}},
            {"N320", 542080, 13437, {-60, 50, -90, 180}},
            {"N640", 2140702, 52654, {-60, 50, -90, 180}},
        };

        for (const auto& t : tests) {
            log << "Test " + t.grid + " (global)" << std::endl;
            repres::RepresentationHandle global(key::grid::Grid::lookup(t.grid).representation());
            EXPECT(global->numberOfPoints() == t.numberOfPoints);

            log << "Test " + t.grid + " (cropped)" << std::endl;
            repres::RepresentationHandle local(global->croppedRepresentation(t.bbox));
            EXPECT(local->numberOfPoints() == t.numberOfCroppedPoints);
        }
    }
#endif

    log.precision(old);
}


}  // namespace unit
}  // namespace tests
}  // namespace mir


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
