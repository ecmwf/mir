/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include <string>
#include "eckit/log/Log.h"
#include "eckit/testing/Test.h"
#include "mir/namedgrids/NamedGrid.h"
#include "mir/repres/Representation.h"
#include "mir/util/BoundingBox.h"

//define EXPECTV(a) log << "\tEXPECT(" << #a <<")" << std::endl; EXPECT(a)


namespace mir {
namespace tests {
namespace unit {


CASE("NamedGrid") {

    using util::BoundingBox;

    auto& log = eckit::Log::info();
    auto old = log.precision(16);


    struct test_t {
        const std::string gridname;
        size_t numberOfPOints;
        BoundingBox bbox;
    };

    std::vector<test_t> tests {
        {"N128",     88838, BoundingBox( 90,         0,      -90,       359.297)},
        {"F320",    819200, BoundingBox( 56,       -16.5,     50.5,      -3)},
        {"N320",    542080, BoundingBox(-60,        50,      -90,       180)},
        {"O320",    421120, BoundingBox( 58,      -130,       25,       -60)},
        {"O320",    421120, BoundingBox(-25,       135,      -47,       179)},
        {"O320",    421120, BoundingBox( 12,       -81,      -50,       -35)},
        {"O320",    421120, BoundingBox( 44,        91,       21,       143)},
        {"O320",    421120, BoundingBox( 60,        23,       35,        80)},
        {"O640",   1661440, BoundingBox( 52,         7,       43,        27.7)},
        {"N640",   2140702, BoundingBox(-60,        50,      -90,       180)},
        {"O640",   1661440, BoundingBox( 58,      -130,       25,       -60)},
        {"O640",   1661440, BoundingBox(-25,       135,      -47,       179)},
        {"F1280", 13107200, BoundingBox( 75,       -30,       25,        60)},
        {"O1280",  6599680, BoundingBox( 12,       -81,      -50,       -35)},
        {"O1280",  6599680, BoundingBox( 37.6025, -114.8907,  27.7626, -105.1875)},
        {"O1280",  6599680, BoundingBox( 90,         0,      -90,       359.929)},
        {"O1280",  6599680, BoundingBox(-25,       135,      -47,       179)},
    };

    SECTION("Area of Gaussian grids numberOfPoints") {
        for (const auto& t : tests) {
            repres::RepresentationHandle r(namedgrids::NamedGrid::lookup(t.gridname).representation());
            EXPECT(r->numberOfPoints() == t.numberOfPOints);
        }
    }

    log.precision(old);
}


} // namespace unit
} // namespace tests
} // namespace mir


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv, false);
}
