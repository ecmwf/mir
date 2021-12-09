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
#include "mir/repres/gauss/GaussianIterator.h"

// define EXPECTV(a) log << "\tEXPECT(" << #a <<")" << std::endl; EXPECT(a)


namespace mir {
namespace tests {
namespace unit {

bool nearby(const Point2& a, const Point2& b) {
    return Point2::distance2(a, b) < 0.000001;
}

CASE("Gaussian Iterator") {

    struct test_t {
        const std::string grid;
        size_t numberOfPoints;
        size_t numberOfCroppedPoints;
        util::BoundingBox bbox;
        eckit::geometry::Point2 globalP1;
        eckit::geometry::Point2 globalP2;
        eckit::geometry::Point2 localP1;
        eckit::geometry::Point2 localP2;
    };

    auto& log = Log::info();
    auto old  = log.precision(16);

    util::BoundingBox globe{90.0,-180.0,-90.0,180.0};

    std::vector<test_t> tests{
        {"F128", 131072, 858, {80.351, -150.46875, 79.648, 150.46875}, {89.4628,0}, {89.4628,0.703125}, {80.3502,-150.469}, {80.3502,-149.766}},
        {"F128", 131072, 429, {80.350, -150.46875, 79.648, 150.46875}, {89.4628,0}, {89.4628,0.703125}, {79.6485,-150.469}, {79.6485,-149.766}},
        {"F320", 819200, 912, {56, -16.5, 50.5, -3}, {89.7849,0}, {89.7849,0.28125}, {55.7845,-16.3125}, {55.7845,-16.0312}},
        {"F1280", 13107200, 910080, {75, -30, 25, 60}, {89.9462,0}, {89.9462,0.0703125}, {74.9736,-29.9531}, {74.9736,-29.8828}},
        {"O320", 421120, 16133, {58, -130, 25, -60},{89.7849,0},{89.7849,18},{57.7517,-129.328},{57.7517,-128.571}},
        {"O320", 421120, 7511, {-25, 135, -47, 179},{89.7849,0},{89.7849,18},{-25.1522,135.191},{-25.1522,135.574}},
        {"O320", 421120, 28101, {12, -81, -50, -35},{89.7849,0},{89.7849,18},{11.9438,-80.7447},{11.9438,-80.4255}},
        {"O320", 421120, 9877, {44, 91, 21, 143},{89.7849,0},{89.7849,18},{43.9812,91.0714},{43.9812,91.6071}},
        {"O320", 421120, 8743, {60, 23, 35, 80},{89.7849,0},{89.7849,18},{60,23.5135},{60,24.3243}},
        {"O640", 1661440, 4512, {52, 7, 43, 27.7},{89.8924,0},{89.8924,18},{51.9406,7.2},{51.9406,7.52727}},
        {"O640", 1661440, 63842, {58, -130, 25, -60},{89.8924,0},{89.8924,18},{57.9852,-129.957},{57.9852,-129.569}},
        {"O640", 1661440, 29674, {-25, 135, -47, 179},{89.8924,0},{89.8924,18},{-25.0918,135},{-25.0918,135.193}},
        {"O1280", 6599680, 442460, {12, -81, -50, -35},{89.9462,0},{89.9462,18},{11.9859,-80.9515},{11.9859,-80.8707}},
        {"O1280", 6599680, 12370, {37.6025, -114.8907, 27.7626, -105.1875},{89.9462,0},{89.9462,18},{37.5747,-114.84},{37.5747,-114.72}},
        {"O1280", 6599680, 6599646, {90, 0, -90, 359.929},{89.9462,0},{89.9462,18},{89.9462,0},{89.9462,18}},
        {"O1280", 6599680, 118210, {-25, 135, -47, 179},{89.9462,0},{89.9462,18},{-25.0615,135},{-25.0615,135.097}},
        {"N128", 88838, 88838, {90, 0, -90, 359.297},{89.4628,0},{89.4628,20},{89.4628,0},{89.4628,20}},
        {"N320", 542080, 13437, {-60, 50, -90, 180},{89.7849,0},{89.7849,20},{-60.281,50.1333},{-60.281,50.6667}},
        {"N640", 2140702, 52654, {-60, 50, -90, 180},{89.8924,0},{89.8924,20},{-60.0937,50.1333},{-60.0937,50.4}},
    };

    for (const auto& t : tests) {
        log << "Test " + t.grid + " (global)" << std::endl;
        repres::RepresentationHandle global(key::grid::Grid::lookup(t.grid).representation());
        {
            repres::Iterator* it = (repres::Iterator*) global->iterator();
            it->next();
            EXPECT(globe.contains(it->pointRotated()));
            EXPECT(nearby(it->pointRotated(), t.globalP1));
            it->next();
            EXPECT(globe.contains(it->pointRotated()));
            EXPECT(nearby(it->pointRotated(), t.globalP2));
        }

        log << "Test " + t.grid + " (cropped)" << std::endl;
        repres::RepresentationHandle local(global->croppedRepresentation(t.bbox));
        {
            repres::Iterator* it = (repres::Iterator*) local->iterator();
            it->next();
            EXPECT(t.bbox.contains(it->pointRotated()));
            EXPECT(nearby(it->pointRotated(), t.localP1));
            it->next();
            EXPECT(t.bbox.contains(it->pointRotated()));
            EXPECT(nearby(it->pointRotated(), t.localP2));
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
